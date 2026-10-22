"""Deterministic models of the DirectX COM interfaces GP4 calls (Direct3D 8).

GP4's renderer reaches Direct3D through COM objects: ``mov eax,[device]; mov ecx,[eax];
push ...; push eax; call [ecx+0xc8]``. In the oracle such a call used to jump through a
generated buffer into unmapped memory, so most of R2 could not be checked at T1. Here every
modelled interface gets a synthetic vtable whose slots are trap addresses in the oracle's trap
page; a call lands on its trap and runs the model below, which

  * logs ``Interface::Method`` with ``this`` and every argument (stack addresses normalised),
    plus a content hash of every input structure the real method reads (matrices, viewports,
    materials, lights, rectangles, vertex/index data of the *UP draws, shader tokens, ...),
  * returns a seeded result: HRESULTs succeed in 7 of 8 calls (failure codes per method),
  * writes seeded outputs (descriptions, caps, display modes, handles), creates child objects
    with consistent state (CreateTexture -> a texture of that size and format; GetSurfaceLevel
    -> the same surface every time) and hands out lock buffers from the modelled heap (so what
    the caller writes into a locked vertex buffer is compared like any other memory effect),
  * pops the stdcall frame (``this`` plus the arguments) and returns to the caller.

The original's and the reconstruction's call logs must be identical (gp4re/stubs.py), so a
reconstruction that sets a different render state, passes a different matrix or draws
different vertices fails even though the model ignores the values. Calls the model cannot
represent faithfully (QueryInterface, private data reads, patches, a RGNDATA dirty region)
end the run as external.

Layouts declare an object with the pointer fill ``com=<Interface>`` (for example a global
``{"addr": "0xb2e808", "type": "ptr", "size": 16, "fill": "com=IDirect3DDevice8"}``): a
buffer whose first dword is that interface's synthetic vtable. Its state (size, format,
length, ...) is seeded on first use. Method tables follow the DirectX 8 SDK headers
(d3d8.h); argument counts exclude ``this``.
"""
from __future__ import annotations

import hashlib
import random
import struct

from unicorn import UcError
from unicorn import x86_const as X

COM_TRAPS = 0x7FF04000          # TRAP + 0x4000: one 4-byte trap address per (interface, method)
COM_VTABLES = 0x7FF08000        # TRAP + 0x8000: the synthetic vtables (dwords = trap addresses)
STRIDE = 0x200                  # 128 methods per interface
MAX_LOCK = 0x00100000           # largest lock buffer the model hands out
MAX_IN = 0x00100000             # largest input structure hashed

S_OK, S_FALSE = 0, 1
E_FAIL, E_OUTOFMEMORY = 0x80004005, 0x8007000E
D3DERR_INVALIDCALL, D3DERR_NOTAVAILABLE = 0x8876086C, 0x8876086A
D3DERR_OUTOFVIDEOMEMORY, D3DERR_DEVICELOST, D3DERR_DEVICENOTRESET = 0x8876017C, 0x88760868, 0x88760869
D3DERR_WASSTILLDRAWING = 0x8876021C
FAILS = (D3DERR_INVALIDCALL,)
CREATE_FAILS = (D3DERR_INVALIDCALL, D3DERR_OUTOFVIDEOMEMORY, E_OUTOFMEMORY)

# D3DFORMAT -> bytes per pixel, or -block size for the 4x4-block DXT formats
FOURCC = lambda s: int.from_bytes(s.encode(), "little")
FORMATS = {20: 3, 21: 4, 22: 4, 23: 2, 24: 2, 25: 2, 26: 2, 27: 1, 28: 1, 29: 2, 30: 2, 41: 1, 50: 1, 51: 2,
           52: 1, 60: 2, 61: 2, 62: 4, 63: 4, 64: 4, 67: 4, 70: 2, 71: 4, 73: 2, 75: 4, 77: 4, 79: 4, 80: 2,
           FOURCC("DXT1"): -8, FOURCC("DXT2"): -16, FOURCC("DXT3"): -16, FOURCC("DXT4"): -16, FOURCC("DXT5"): -16}
COMMON_FORMATS = (21, 22, 23, 24, 25, 26, 28, 50, 51, FOURCC("DXT1"), FOURCC("DXT3"), FOURCC("DXT5"))
D3DFMT_VERTEXDATA, D3DFMT_INDEX16, D3DFMT_INDEX32 = 100, 101, 102


class Unsupported(Exception):
    """Re-exported by gp4re.stubs: the model cannot represent this call faithfully."""


def _verts(ptype: int, count: int) -> int:
    n = {1: count, 2: 2 * count, 3: count + 1, 4: 3 * count, 5: count + 2, 6: count + 2}.get(ptype)
    if n is None:
        raise Unsupported(f"primitive type {ptype} is not a D3DPRIMITIVETYPE")
    return n


class M:
    """One method: name, argument count (without this), result kind, hashed inputs, effect."""

    def __init__(self, name, n, ret="hr", ins=(), eff=None, fails=FAILS, rate=8, unsupported=None):
        self.name, self.n, self.ret, self.ins, self.eff = name, n, ret, ins, eff
        self.fails, self.rate, self.unsupported = fails, rate, unsupported


def _const(v):
    return lambda model, this, a: v


# ---------------------------------------------------------------- effects
def put_obj(i, iface, state=None):
    """Write a new object of iface to *args[i] (NULL when the call fails)."""
    def eff(model, this, a, ok):
        if not a[i]:
            return D3DERR_INVALIDCALL if ok else None
        model.put(a[i], struct.pack("<I", model.new(iface, **(state(model, this, a) if state else {})) if ok else 0))
    return eff


def put_child(i, iface, key, shared=False):
    """Write the persistent child object key(this, args) of this (a surface level, a back buffer);
    shared: one object for the whole run (the device every resource reports, the IDirect3D8)."""
    def eff(model, this, a, ok):
        if not a[i]:
            return D3DERR_INVALIDCALL if ok else None
        if not ok:
            model.put(a[i], bytes(4))
            return None
        k = key(model, this, a)
        if k is None:
            model.put(a[i], bytes(4))
            return D3DERR_INVALIDCALL
        model.put(a[i], struct.pack("<I", model.child(0 if shared else this, k, iface)))
    return eff


def put_u32(i, gen):
    def eff(model, this, a, ok):
        if ok and a[i]:
            model.put(a[i], struct.pack("<I", gen(model, this, a) & 0xFFFFFFFF))
    return eff


def put_bytes(i, gen):
    def eff(model, this, a, ok):
        if ok and a[i]:
            model.put(a[i], gen(model, this, a))
    return eff


def both(*effs):
    def eff(model, this, a, ok):
        out = None
        for e in effs:
            r = e(model, this, a, ok)
            out = r if r is not None else out
        return out
    return eff


def seeded_u32(*choices):
    return lambda model, this, a: model.rng.choice(choices)


def floats(n, lo=-100.0, hi=100.0):
    return lambda model, this, a: b"".join(struct.pack("<f", model.rng.uniform(lo, hi)) for _ in range(n))


def handle(model, this, a):
    model.next_handle += model.rng.randint(1, 8)
    return model.next_handle


def display_mode(model, this, a):
    r = model.rng
    w, h = r.choice(((640, 480), (800, 600), (1024, 768), (1280, 1024), (1600, 1200)))
    return struct.pack("<4I", w, h, r.choice((0, 60, 75, 85)), r.choice((22, 23, 24, 25, 21)))


def viewport(model, this, a):
    r = model.rng
    return struct.pack("<4I2f", r.randrange(64), r.randrange(64), r.choice((320, 640, 800, 1024)),
                       r.choice((240, 480, 600, 768)), 0.0, 1.0)


def caps8(model, this, a):
    """D3DCAPS8 (212 bytes): random capability bits, plausible limits."""
    r = model.rng
    bits = lambda: r.getrandbits(32)
    out = [r.choice((1, 2)), 0] + [bits() for _ in range(20)]          # DeviceType .. LineCaps
    out += [r.choice((256, 512, 1024, 2048, 4096)), r.choice((256, 512, 1024, 2048, 4096)), r.choice((0, 256)),
            r.choice((0, 8192)), r.choice((0, 8, 2048)), r.choice((1, 2, 4, 8, 16))]
    f = [r.choice((0.0, 1e10)), -r.choice((2048.0, 1e8)), -r.choice((2048.0, 1e8)), r.choice((2048.0, 1e8)),
         r.choice((2048.0, 1e8)), r.choice((0.0, 0.5, 1.0))]                # MaxVertexW .. ExtentsAdjust
    tail = [bits(), bits(), bits(), r.randint(1, 8), r.randint(1, 8), bits(), r.randint(0, 8), r.randint(0, 6),
            r.randint(0, 4), r.randint(0, 255)]                               # StencilCaps .. MaxVertexBlendMatrixIndex
    tail2 = [r.choice((0xFFFF, 0xFFFFF)), r.choice((0xFFFF, 0xFFFFFF)), r.randint(1, 16), r.choice((255, 256, 1024)),
             0xFFFE0000 | r.choice((0, 0x100, 0x101)), r.choice((0, 96)), 0xFFFF0000 | r.choice((0, 0x100, 0x103, 0x104))]
    data = (struct.pack("<28I", *out) + struct.pack("<6f", *f) + struct.pack("<10I", *tail)
            + struct.pack("<f", r.choice((1.0, 64.0))) + struct.pack("<7I", *tail2) + struct.pack("<f", r.choice((1.0, 8.0))))
    assert len(data) == 212
    return data


def adapter_ident(model, this, a):
    """D3DADAPTER_IDENTIFIER8 (1072 bytes) with short NUL-terminated strings."""
    r = model.rng
    drv = f"drv{r.randrange(256):02x}.dll".encode().ljust(512, b"\0")
    desc = f"Display adapter {r.randrange(16)}".encode().ljust(512, b"\0")
    return drv + desc + struct.pack("<Q4I", r.getrandbits(63), r.choice((0x10DE, 0x1002, 0x8086, 0x121A)),
                                    r.getrandbits(16), r.getrandbits(32), r.randrange(16)) \
        + r.randbytes(16) + struct.pack("<I", r.randrange(3)) + bytes(4)


# ---------------------------------------------------------------- object state
def _level(st, level):
    return max(1, st["w"] >> level), max(1, st["h"] >> level)


def _levels(st):
    if st.get("levels"):
        return st["levels"]
    n, m = 1, max(st["w"], st["h"], st.get("d", 1))
    while m > 1:
        m, n = m >> 1, n + 1
    return n


def surface_desc(st, w, h, rtype=1):
    """D3DSURFACE_DESC (D3D8, 32 bytes)."""
    size = _pitch(st["fmt"], w, h)[0] * _pitch(st["fmt"], w, h)[1] if st["fmt"] in FORMATS else 0
    return struct.pack("<8I", st["fmt"], rtype, st["usage"], st["pool"], size, 0, w, h)


def _pitch(fmt, w, h=1):
    bpp = FORMATS.get(fmt)
    if bpp is None:
        raise Unsupported(f"lock of D3DFORMAT {fmt:#x}: unknown pixel size")
    if bpp < 0:
        return max(1, (w + 3) // 4) * -bpp, max(1, (h + 3) // 4)
    return w * bpp, h


def tex_state(model, this, a, w=0, h=1, d=None, lv=2, usage=3, fmt=4, pool=5):
    st = {"w": max(1, a[w]), "h": max(1, a[h]) if h is not None else max(1, a[w]), "levels": a[lv],
          "usage": a[usage], "fmt": a[fmt], "pool": a[pool]}
    if d is not None:
        st["d"] = max(1, a[d])
    return st


# ---------------------------------------------------------------- lock helpers
def lock_rect(level_arg, out_arg, rect_arg, face_arg=None):
    def eff(model, this, a, ok):
        st = model.state(this)
        if st.get("parent"):                                     # a surface of a texture level
            owner, key = st["parent"]
            fmt, (w, h) = model.state(owner)["fmt"], (st["w"], st["h"])
        elif level_arg is None:                                  # a stand-alone surface
            owner, key, fmt, (w, h) = this, (0, 0), st["fmt"], (st["w"], st["h"])
        else:
            if a[level_arg] >= _levels(st):
                return D3DERR_INVALIDCALL
            owner, key = this, (a[face_arg] if face_arg is not None else 0, a[level_arg])
            fmt, (w, h) = st["fmt"], _level(st, a[level_arg])
        if not a[out_arg]:
            return D3DERR_INVALIDCALL
        if not ok:
            return None
        pitch, rows = _pitch(fmt, w, h)
        buf, pitch = model.lockbuf(owner, key, pitch, rows)
        off = 0
        if a[rect_arg]:
            left, top, right, bottom = struct.unpack("<4i", model.read(a[rect_arg], 16))
            if not (0 <= left < right <= w and 0 <= top < bottom <= h):
                return D3DERR_INVALIDCALL
            bpp = FORMATS[fmt]
            off = (top // 4) * pitch + (left // 4) * -bpp if bpp < 0 else top * pitch + left * bpp
        model.put(a[out_arg], struct.pack("<iI", pitch, buf + off))
    return eff


def lock_box(level_arg, out_arg, box_arg):
    def eff(model, this, a, ok):
        st = model.state(this)
        if st.get("parent"):
            owner, key = st["parent"]
            fmt, (w, h, d) = model.state(owner)["fmt"], (st["w"], st["h"], st["d"])
        else:
            level = a[level_arg] if level_arg is not None else 0
            if level >= _levels(st):
                return D3DERR_INVALIDCALL
            owner, key = this, (0, level)
            fmt, (w, h), d = st["fmt"], _level(st, level), max(1, st["d"] >> level)
        if not a[out_arg]:
            return D3DERR_INVALIDCALL
        if not ok:
            return None
        row, rows = _pitch(fmt, w, h)
        buf, row = model.lockbuf(owner, key, row, rows * d)
        off = 0
        if a[box_arg]:
            left, top, right, bottom, front, back = struct.unpack("<6I", model.read(a[box_arg], 24))
            if not (left < right <= w and top < bottom <= h and front < back <= d) or FORMATS[fmt] < 0:
                return D3DERR_INVALIDCALL
            off = front * row * rows + top * row + left * FORMATS[fmt]
        model.put(a[out_arg], struct.pack("<iiI", row, row * rows, buf + off))
    return eff


def lock_buffer(out_arg):
    def eff(model, this, a, ok):
        st = model.state(this)
        off, size = a[0], a[1] or max(0, st["length"] - a[0])
        if off + size > st["length"] or not a[out_arg]:
            return D3DERR_INVALIDCALL
        if not ok:
            return None
        buf, _ = model.lockbuf(this, 0, st["length"], 1)
        model.put(a[out_arg], struct.pack("<I", buf + off))
    return eff


# ---------------------------------------------------------------- interfaces
IUNKNOWN = [M("QueryInterface", 2, unsupported="QueryInterface results are not modelled"),
            M("AddRef", 0, ret="ulong"), M("Release", 0, ret="ulong")]

RESOURCE = IUNKNOWN + [
    M("GetDevice", 1, eff=put_child(0, "IDirect3DDevice8", lambda m, t, a: "device", shared=True), fails=()),
    M("SetPrivateData", 4, ins=((0, 16), (1, lambda a: 0 if a[3] & 1 else a[2]))),
    M("GetPrivateData", 3, unsupported="private data reads are not modelled"),
    M("FreePrivateData", 1, ins=((0, 16),)),
    M("SetPriority", 1, ret="u32", eff=None), M("GetPriority", 0, ret="u32"),
    M("PreLoad", 0, ret="void"), M("GetType", 0, ret="type")]

BASETEXTURE = RESOURCE + [M("SetLOD", 1, ret="u32"), M("GetLOD", 0, ret="u32"), M("GetLevelCount", 0, ret="levels")]


def _level_key(level_arg, face_arg=None):
    def key(model, this, a):
        if a[level_arg] >= _levels(model.state(this)):
            return None
        return (a[face_arg] if face_arg is not None else 0, a[level_arg])
    return key


def _desc_eff(out, level_arg, rtype, volume=False):
    def eff(model, this, a, ok):
        st = model.state(this)
        if a[level_arg] >= _levels(st) or not a[out]:
            return D3DERR_INVALIDCALL
        if ok:
            w, h = _level(st, a[level_arg])
            if volume:
                d = max(1, st["d"] >> a[level_arg])
                row, rows = _pitch(st["fmt"], w, h)
                model.put(a[out], struct.pack("<8I", st["fmt"], 2, st["usage"], st["pool"], row * rows * d, w, h, d))
            else:
                model.put(a[out], surface_desc(st, w, h, rtype))
    return eff


TEXTURE = BASETEXTURE + [
    M("GetLevelDesc", 2, eff=_desc_eff(1, 0, 1)),
    M("GetSurfaceLevel", 2, eff=put_child(1, "IDirect3DSurface8", _level_key(0))),
    M("LockRect", 4, ins=((2, 16),), eff=lock_rect(0, 1, 2), fails=(D3DERR_INVALIDCALL, D3DERR_WASSTILLDRAWING)),
    M("UnlockRect", 1), M("AddDirtyRect", 1, ins=((0, 16),))]

CUBETEXTURE = BASETEXTURE + [
    M("GetLevelDesc", 2, eff=_desc_eff(1, 0, 1)),
    M("GetCubeMapSurface", 3, eff=put_child(2, "IDirect3DSurface8", _level_key(1, 0))),
    M("LockRect", 5, ins=((3, 16),), eff=lock_rect(1, 2, 3, face_arg=0)),
    M("UnlockRect", 2), M("AddDirtyRect", 2, ins=((1, 16),))]

VOLUMETEXTURE = BASETEXTURE + [
    M("GetLevelDesc", 2, eff=_desc_eff(1, 0, 4, volume=True)),
    M("GetVolumeLevel", 2, eff=put_child(1, "IDirect3DVolume8", _level_key(0))),
    M("LockBox", 4, ins=((2, 24),), eff=lock_box(0, 1, 2)),
    M("UnlockBox", 1), M("AddDirtyBox", 1, ins=((0, 24),))]


def _vb_desc(model, this, a):
    st = model.state(this)
    return struct.pack("<6I", D3DFMT_VERTEXDATA, 6, st["usage"], st["pool"], st["length"], st["fvf"])


def _ib_desc(model, this, a):
    st = model.state(this)
    return struct.pack("<5I", st["fmt"], 7, st["usage"], st["pool"], st["length"])


VERTEXBUFFER = RESOURCE + [M("Lock", 4, eff=lock_buffer(2), fails=(D3DERR_INVALIDCALL, D3DERR_WASSTILLDRAWING)),
                           M("Unlock", 0), M("GetDesc", 1, eff=put_bytes(0, _vb_desc))]
INDEXBUFFER = RESOURCE + [M("Lock", 4, eff=lock_buffer(2), fails=(D3DERR_INVALIDCALL, D3DERR_WASSTILLDRAWING)),
                          M("Unlock", 0), M("GetDesc", 1, eff=put_bytes(0, _ib_desc))]

_SURF_COMMON = IUNKNOWN + [
    M("GetDevice", 1, eff=put_child(0, "IDirect3DDevice8", lambda m, t, a: "device", shared=True), fails=()),
    M("SetPrivateData", 4, ins=((0, 16), (1, lambda a: 0 if a[3] & 1 else a[2]))),
    M("GetPrivateData", 3, unsupported="private data reads are not modelled"),
    M("FreePrivateData", 1, ins=((0, 16),)),
    M("GetContainer", 2, unsupported="GetContainer results are not modelled")]
SURFACE = _SURF_COMMON + [
    M("GetDesc", 1, eff=put_bytes(0, lambda m, t, a: surface_desc(m.state(t), m.state(t)["w"], m.state(t)["h"]))),
    M("LockRect", 3, ins=((1, 16),), eff=lock_rect(None, 0, 1), fails=(D3DERR_INVALIDCALL, D3DERR_WASSTILLDRAWING)),
    M("UnlockRect", 0)]
VOLUME = _SURF_COMMON + [
    M("GetDesc", 1, eff=put_bytes(0, lambda m, t, a: struct.pack(
        "<8I", m.state(t)["fmt"], 2, m.state(t)["usage"], m.state(t)["pool"], 0, m.state(t)["w"], m.state(t)["h"],
        m.state(t)["d"]))),
    M("LockBox", 3, ins=((1, 24),), eff=lock_box(None, 0, 1)), M("UnlockBox", 0)]

SWAPCHAIN = IUNKNOWN + [
    M("Present", 4, ins=((0, 16), (1, 16)), eff=lambda m, t, a, ok: m.no_region(a[3])),
    M("GetBackBuffer", 3, eff=put_child(2, "IDirect3DSurface8", lambda m, t, a: ("back", a[0], a[1])))]

DIRECT3D8 = IUNKNOWN + [
    M("RegisterSoftwareDevice", 1, unsupported="software devices are not modelled"),
    M("GetAdapterCount", 0, ret="u32:1,1,2"),
    M("GetAdapterIdentifier", 3, eff=put_bytes(2, adapter_ident)),
    M("GetAdapterModeCount", 1, ret="u32:0,1,4,12,24"),
    M("EnumAdapterModes", 3, eff=put_bytes(2, display_mode)),
    M("GetAdapterDisplayMode", 2, eff=put_bytes(1, display_mode)),
    M("CheckDeviceType", 5, fails=(D3DERR_NOTAVAILABLE, D3DERR_INVALIDCALL), rate=3),
    M("CheckDeviceFormat", 6, fails=(D3DERR_NOTAVAILABLE,), rate=3),
    M("CheckDeviceMultiSampleType", 5, fails=(D3DERR_NOTAVAILABLE,), rate=3),
    M("CheckDepthStencilMatch", 5, fails=(D3DERR_NOTAVAILABLE,), rate=3),
    M("GetDeviceCaps", 3, eff=put_bytes(2, caps8)),
    M("GetAdapterMonitor", 1, ret="u32:0x10001,0x10003"),
    M("CreateDevice", 6, ins=((4, 52),), eff=put_obj(5, "IDirect3DDevice8"),
      fails=(D3DERR_INVALIDCALL, D3DERR_NOTAVAILABLE, D3DERR_OUTOFVIDEOMEMORY))]


def _const_tokens(a_index, end):
    def size(a):
        return ("tokens", a[a_index], end)
    return size


DEVICE8 = IUNKNOWN + [
    M("TestCooperativeLevel", 0, fails=(D3DERR_DEVICELOST, D3DERR_DEVICENOTRESET)),
    M("GetAvailableTextureMem", 0, ret="u32:0x00400000,0x01000000,0x02000000,0x04000000,0x08000000"),
    M("ResourceManagerDiscardBytes", 1),
    M("GetDirect3D", 1, eff=put_child(0, "IDirect3D8", lambda m, t, a: "d3d", shared=True), fails=()),
    M("GetDeviceCaps", 1, eff=put_bytes(0, caps8)),
    M("GetDisplayMode", 1, eff=put_bytes(0, display_mode)),
    M("GetCreationParameters", 1, eff=put_bytes(0, lambda m, t, a: struct.pack(
        "<4I", 0, m.rng.choice((1, 2)), 0x10010, m.rng.choice((0x20, 0x40, 0x50))))),
    M("SetCursorProperties", 3),
    M("SetCursorPosition", 3, ret="void"),
    M("ShowCursor", 1, ret="u32:0,1"),
    M("CreateAdditionalSwapChain", 2, ins=((0, 52),), eff=put_obj(1, "IDirect3DSwapChain8"), fails=CREATE_FAILS),
    M("Reset", 1, ins=((0, 52),), fails=(D3DERR_INVALIDCALL, D3DERR_DEVICELOST, D3DERR_OUTOFVIDEOMEMORY)),
    M("Present", 4, ins=((0, 16), (1, 16)), eff=lambda m, t, a, ok: m.no_region(a[3]), fails=(D3DERR_DEVICELOST,)),
    M("GetBackBuffer", 3, eff=put_child(2, "IDirect3DSurface8", lambda m, t, a: ("back", a[0], a[1]))),
    M("GetRasterStatus", 1, eff=put_bytes(0, lambda m, t, a: struct.pack("<2I", m.rng.randrange(2), m.rng.randrange(1200)))),
    M("SetGammaRamp", 2, ret="void", ins=((1, 1536),)),
    M("GetGammaRamp", 1, ret="void", eff=lambda m, t, a, ok: m.put(a[0], m.rng.randbytes(1536)) if a[0] else None),
    M("CreateTexture", 7, eff=put_obj(6, "IDirect3DTexture8", lambda m, t, a: tex_state(m, t, a)), fails=CREATE_FAILS),
    M("CreateVolumeTexture", 8, eff=put_obj(7, "IDirect3DVolumeTexture8",
                                            lambda m, t, a: tex_state(m, t, a, 0, 1, 2, 3, 4, 5, 6)), fails=CREATE_FAILS),
    M("CreateCubeTexture", 6, eff=put_obj(5, "IDirect3DCubeTexture8",
                                          lambda m, t, a: tex_state(m, t, a, 0, None, None, 1, 2, 3, 4)), fails=CREATE_FAILS),
    M("CreateVertexBuffer", 5, eff=put_obj(4, "IDirect3DVertexBuffer8", lambda m, t, a: {
        "length": a[0], "usage": a[1], "fvf": a[2], "pool": a[3]}), fails=CREATE_FAILS),
    M("CreateIndexBuffer", 5, eff=put_obj(4, "IDirect3DIndexBuffer8", lambda m, t, a: {
        "length": a[0], "usage": a[1], "fmt": a[2], "pool": a[3]}), fails=CREATE_FAILS),
    M("CreateRenderTarget", 6, eff=put_obj(5, "IDirect3DSurface8", lambda m, t, a: {
        "w": max(1, a[0]), "h": max(1, a[1]), "fmt": a[2], "usage": 1, "pool": 0}), fails=CREATE_FAILS),
    M("CreateDepthStencilSurface", 5, eff=put_obj(4, "IDirect3DSurface8", lambda m, t, a: {
        "w": max(1, a[0]), "h": max(1, a[1]), "fmt": a[2], "usage": 2, "pool": 0}), fails=CREATE_FAILS),
    M("CreateImageSurface", 4, eff=put_obj(3, "IDirect3DSurface8", lambda m, t, a: {
        "w": max(1, a[0]), "h": max(1, a[1]), "fmt": a[2], "usage": 0, "pool": 2}), fails=CREATE_FAILS),
    M("CopyRects", 5, ins=((1, lambda a: 16 * a[2]), (4, lambda a: 8 * a[2]))),
    M("UpdateTexture", 2),
    M("GetFrontBuffer", 1),
    M("SetRenderTarget", 2),
    M("GetRenderTarget", 1, eff=put_child(0, "IDirect3DSurface8", lambda m, t, a: "rendertarget")),
    M("GetDepthStencilSurface", 1, eff=put_child(0, "IDirect3DSurface8", lambda m, t, a: "depthstencil"),
      fails=(D3DERR_NOTAVAILABLE,)),
    M("BeginScene", 0), M("EndScene", 0),
    M("Clear", 6, ins=((1, lambda a: 16 * a[0]),)),
    M("SetTransform", 2, ins=((1, 64),)),
    M("GetTransform", 2, eff=put_bytes(1, floats(16))),
    M("MultiplyTransform", 2, ins=((1, 64),)),
    M("SetViewport", 1, ins=((0, 24),)),
    M("GetViewport", 1, eff=put_bytes(0, viewport)),
    M("SetMaterial", 1, ins=((0, 68),)),
    M("GetMaterial", 1, eff=put_bytes(0, floats(17, 0.0, 1.0))),
    M("SetLight", 2, ins=((1, 104),)),
    M("GetLight", 2, eff=put_bytes(1, lambda m, t, a: struct.pack("<I", m.rng.randint(1, 3)) + floats(25)(m, t, a))),
    M("LightEnable", 2),
    M("GetLightEnable", 2, eff=put_u32(1, seeded_u32(0, 1))),
    M("SetClipPlane", 2, ins=((1, 16),)),
    M("GetClipPlane", 2, eff=put_bytes(1, floats(4))),
    M("SetRenderState", 2),
    M("GetRenderState", 2, eff=put_u32(1, lambda m, t, a: m.rng.choice((0, 1, 2, 3, m.rng.getrandbits(32))))),
    M("BeginStateBlock", 0),
    M("EndStateBlock", 1, eff=put_u32(0, handle)),
    M("ApplyStateBlock", 1), M("CaptureStateBlock", 1), M("DeleteStateBlock", 1),
    M("CreateStateBlock", 2, eff=put_u32(1, handle)),
    M("SetClipStatus", 1, ins=((0, 8),)),
    M("GetClipStatus", 1, eff=put_bytes(0, lambda m, t, a: struct.pack("<2I", m.rng.getrandbits(32), m.rng.getrandbits(32)))),
    M("GetTexture", 2, eff=put_child(1, "IDirect3DTexture8", lambda m, t, a: ("stage", a[0]))),
    M("SetTexture", 2),
    M("GetTextureStageState", 3, eff=put_u32(2, lambda m, t, a: m.rng.choice((0, 1, 2, 3, 4, m.rng.getrandbits(32))))),
    M("SetTextureStageState", 3),
    M("ValidateDevice", 1, eff=put_u32(0, seeded_u32(1, 1, 2)), fails=(D3DERR_NOTAVAILABLE, D3DERR_INVALIDCALL), rate=4),
    M("GetInfo", 3, unsupported="GetInfo results are not modelled"),
    M("SetPaletteEntries", 2, ins=((1, 1024),)),
    M("GetPaletteEntries", 2, eff=put_bytes(1, lambda m, t, a: m.rng.randbytes(1024))),
    M("SetCurrentTexturePalette", 1),
    M("GetCurrentTexturePalette", 1, eff=put_u32(0, lambda m, t, a: m.rng.randrange(256))),
    M("DrawPrimitive", 3),
    M("DrawIndexedPrimitive", 5),
    M("DrawPrimitiveUP", 4, ins=((2, lambda a: _verts(a[0], a[1]) * a[3]),)),
    M("DrawIndexedPrimitiveUP", 8, ins=((4, lambda a: _verts(a[0], a[3]) * (2 if a[5] == D3DFMT_INDEX16 else 4)),
                                         (6, lambda a: (a[1] + a[2]) * a[7]))),
    M("ProcessVertices", 5),
    M("CreateVertexShader", 4, ins=((0, _const_tokens(0, 0xFFFFFFFF)), (1, _const_tokens(1, 0x0000FFFF))),
      eff=put_u32(2, handle)),
    M("SetVertexShader", 1),
    M("GetVertexShader", 1, eff=put_u32(0, handle)),
    M("DeleteVertexShader", 1),
    M("SetVertexShaderConstant", 3, ins=((1, lambda a: 16 * a[2]),)),
    M("GetVertexShaderConstant", 3, eff=lambda m, t, a, ok: m.put(a[1], floats(4 * a[2])(m, t, a)) if ok and a[1] else None),
    M("GetVertexShaderDeclaration", 3, unsupported="shader declaration reads are not modelled"),
    M("GetVertexShaderFunction", 3, unsupported="shader function reads are not modelled"),
    M("SetStreamSource", 3),
    M("GetStreamSource", 3, eff=both(put_child(1, "IDirect3DVertexBuffer8", lambda m, t, a: ("stream", a[0])),
                                     put_u32(2, seeded_u32(12, 16, 24, 28, 32, 36)))),
    M("SetIndices", 2),
    M("GetIndices", 2, eff=both(put_child(0, "IDirect3DIndexBuffer8", lambda m, t, a: "indices"),
                                put_u32(1, seeded_u32(0, 0, 4, 64)))),
    M("CreatePixelShader", 2, ins=((0, _const_tokens(0, 0x0000FFFF)),), eff=put_u32(1, handle)),
    M("SetPixelShader", 1),
    M("GetPixelShader", 1, eff=put_u32(0, handle)),
    M("DeletePixelShader", 1),
    M("SetPixelShaderConstant", 3, ins=((1, lambda a: 16 * a[2]),)),
    M("GetPixelShaderConstant", 3, eff=lambda m, t, a, ok: m.put(a[1], floats(4 * a[2])(m, t, a)) if ok and a[1] else None),
    M("GetPixelShaderFunction", 3, unsupported="shader function reads are not modelled"),
    M("DrawRectPatch", 3, unsupported="patches are not modelled"),
    M("DrawTriPatch", 3, unsupported="patches are not modelled"),
    M("DeletePatch", 1)]

IFACES = {
    "IDirect3D8": DIRECT3D8,
    "IDirect3DDevice8": DEVICE8,
    "IDirect3DSwapChain8": SWAPCHAIN,
    "IDirect3DTexture8": TEXTURE,
    "IDirect3DCubeTexture8": CUBETEXTURE,
    "IDirect3DVolumeTexture8": VOLUMETEXTURE,
    "IDirect3DSurface8": SURFACE,
    "IDirect3DVolume8": VOLUME,
    "IDirect3DVertexBuffer8": VERTEXBUFFER,
    "IDirect3DIndexBuffer8": INDEXBUFFER,
}
assert len(DEVICE8) == 97 and len(DIRECT3D8) == 16 and len(TEXTURE) == 19 and len(VERTEXBUFFER) == 14
assert len(SURFACE) == 11 and len(VOLUME) == 11 and len(SWAPCHAIN) == 5
SEEDED: dict = {}          # interface -> seeded state of a layout-made object (filled by com_dx)
NAMES = list(IFACES)
RESOURCE_TYPE = {"IDirect3DSurface8": 1, "IDirect3DVolume8": 2, "IDirect3DTexture8": 3, "IDirect3DVolumeTexture8": 4,
                 "IDirect3DCubeTexture8": 5, "IDirect3DVertexBuffer8": 6, "IDirect3DIndexBuffer8": 7}


def vtable(name: str) -> int:
    if name not in IFACES:
        raise ValueError(f"unknown COM interface {name!r} (modelled: {', '.join(NAMES)})")
    return COM_VTABLES + NAMES.index(name) * STRIDE


def vtable_image() -> list[tuple[int, bytes]]:
    """(address, bytes) of every synthetic vtable, written into the trap page at machine set-up."""
    return [(COM_VTABLES + k * STRIDE, b"".join(struct.pack("<I", COM_TRAPS + k * STRIDE + 4 * m)
                                                 for m in range(len(IFACES[n])))) for k, n in enumerate(NAMES)]


def method_at(address: int) -> tuple[str, M] | None:
    off = address - COM_TRAPS
    if off < 0 or off >= len(NAMES) * STRIDE or off & 3:
        return None
    k, m = divmod(off, STRIDE)
    methods = IFACES[NAMES[k]]
    return (NAMES[k], methods[m // 4]) if m // 4 < len(methods) else None


def _seeded_state(name: str, rng: random.Random) -> dict:
    if name in SEEDED:
        return SEEDED[name](rng)
    dim = lambda: rng.choice((1, 2, 4, 8, 16, 32, 64, 128, 256))
    fmt, usage, pool = rng.choice(COMMON_FORMATS), rng.choice((0, 0, 1, 0x200)), rng.choice((0, 1, 1, 2))
    if name in ("IDirect3DTexture8", "IDirect3DCubeTexture8"):
        w = dim()
        return {"w": w, "h": w if name == "IDirect3DCubeTexture8" else dim(), "levels": rng.randint(1, 4),
                "fmt": fmt, "usage": usage, "pool": pool}
    if name == "IDirect3DVolumeTexture8":
        return {"w": dim(), "h": dim(), "d": rng.choice((1, 2, 4, 8)), "levels": rng.randint(1, 3), "fmt": rng.choice(
            (21, 22, 23, 28)), "usage": 0, "pool": pool}
    if name == "IDirect3DSurface8":
        return {"w": dim(), "h": dim(), "fmt": fmt, "usage": usage, "pool": pool}
    if name == "IDirect3DVolume8":
        return {"w": dim(), "h": dim(), "d": rng.choice((1, 2, 4)), "fmt": rng.choice((21, 22, 23)), "usage": 0,
                "pool": pool}
    if name == "IDirect3DVertexBuffer8":
        return {"length": rng.choice((32, 64, 256, 1024, 4096, 0x8000)), "usage": rng.choice((0, 8, 0x200, 0x208)),
                "fvf": rng.choice((0x002, 0x004, 0x102, 0x112, 0x142, 0x144, 0x152, 0x1C4, 0x252)), "pool": pool}
    if name == "IDirect3DIndexBuffer8":
        return {"length": rng.choice((32, 64, 256, 1024, 4096, 0x8000)), "usage": rng.choice((0, 8, 0x200, 0x208)),
                "fmt": rng.choice((D3DFMT_INDEX16, D3DFMT_INDEX16, D3DFMT_INDEX32)), "pool": pool}
    return {}


class ComModel:
    """Per-run COM state, owned by gp4re.stubs.ImportStubs (shares its heap, rng and call log)."""

    def __init__(self, stubs):
        self.stubs = stubs
        self.reset()

    def reset(self) -> None:
        self.objs: dict[int, dict] = {}
        self.children: dict[tuple, int] = {}
        self.locks: dict[tuple, tuple[int, int]] = {}
        self.next_handle = 0x00010000
        self.failures = getattr(self, "failures", True)   # spec comfail=off turns injected failures off

    @property
    def rng(self):
        return self.stubs.rng

    # ------------------------------------------------------------ memory
    def read(self, addr: int, n: int) -> bytes:
        try:
            return bytes(self.stubs.uc.mem_read(addr, n))
        except UcError:
            raise Unsupported(f"input structure at {addr:#010x} (+{n:#x}) is not mapped")

    def put(self, addr: int, data: bytes) -> None:
        self.stubs._write(addr, data)

    def no_region(self, p: int) -> None:
        if p:
            raise Unsupported("Present with a RGNDATA dirty region is not modelled")

    # ------------------------------------------------------------ objects
    def iface_of(self, obj: int) -> str | None:
        try:
            vt = struct.unpack("<I", bytes(self.stubs.uc.mem_read(obj, 4)))[0]
        except UcError:
            return None
        k, r = divmod(vt - COM_VTABLES, STRIDE)
        return NAMES[k] if 0 <= k < len(NAMES) and r == 0 else None

    def new(self, iface: str, **state) -> int:
        p = self.stubs._alloc(16, True)
        if not p:
            raise Unsupported("the modelled heap is exhausted")
        self.put(p, struct.pack("<I", vtable(iface)))
        self.objs[p] = {"iface": iface, **state}
        return p

    def state(self, obj: int) -> dict:
        st = self.objs.get(obj)
        if st is None:
            iface = self.iface_of(obj)
            if iface is None:
                raise Unsupported(f"object {obj:#010x} has no modelled COM vtable")
            seed = (self.stubs.seed * 0x9E3779B1 + obj) & 0xFFFFFFFFFFFF
            st = self.objs[obj] = {"iface": iface, **_seeded_state(iface or "", random.Random(seed))}
        return st

    def child(self, parent: int, key, iface: str) -> int:
        k = (parent, key)
        if k not in self.children:
            st = {}
            pst = self.state(parent) if parent else {}
            if isinstance(key, tuple) and len(key) == 2 and "fmt" in pst and iface in ("IDirect3DSurface8", "IDirect3DVolume8"):
                w, h = _level(pst, key[1])
                st = {"w": w, "h": h, "fmt": pst["fmt"], "usage": pst["usage"], "pool": pst["pool"], "parent": (parent, key)}
                if iface == "IDirect3DVolume8":
                    st["d"] = max(1, pst["d"] >> key[1])
            else:      # a stable seed (str hashes are randomised per process)
                h = int.from_bytes(hashlib.sha1(repr((self.stubs.seed, parent, key)).encode()).digest()[:8], "little")
                st = _seeded_state(iface, random.Random(h))
            self.children[k] = self.new(iface, **st)
        return self.children[k]

    def lockbuf(self, owner: int, key, pitch: int, rows: int) -> tuple[int, int]:
        k = (owner, key)
        if k not in self.locks:
            pad = self.rng.choice((0, 0, 0, 16, 64)) if rows > 1 else 0
            size = (pitch + pad) * rows
            if size > MAX_LOCK:
                raise Unsupported(f"lock of {size:#x} bytes exceeds the model's {MAX_LOCK:#x}")
            p = self.stubs._alloc(size, False)
            if not p:
                raise Unsupported("the modelled heap is exhausted")
            self.locks[k] = (p, pitch + pad)
        return self.locks[k]

    # ------------------------------------------------------------ dispatch
    def _hash_in(self, a: list[int], index: int, size) -> str:
        p = a[index]
        if not p:
            return "null"
        n = (size(a) if size.__code__.co_argcount == 1 else size(a, self.read)) if callable(size) else size
        if isinstance(n, tuple) and n[0] == "bytes":          # a structure with nested parts, gathered by size()
            return "mem:" + hashlib.sha1(n[1]).hexdigest()[:16]
        if isinstance(n, tuple):                               # shader token stream up to its end token
            _, start, end = n
            out, off = bytearray(), 0
            while off < MAX_IN:
                tok = self.read(start + off, 4)
                out += tok
                off += 4
                if struct.unpack("<I", tok)[0] == end:
                    break
            else:
                raise Unsupported("shader token stream without an end token")
            data = bytes(out)
        else:
            if n < 0 or n > MAX_IN:
                raise Unsupported(f"input structure of {n} bytes")
            data = self.read(p, n) if n else b""
        return "mem:" + hashlib.sha1(data).hexdigest()[:16]

    def call(self, address: int) -> None:
        found = method_at(address)
        if found is None:
            raise Unsupported(f"call into the COM trap page at {address:#010x} outside a modelled method")
        iface, m = found
        uc = self.stubs.uc
        esp = uc.reg_read(X.UC_X86_REG_ESP)
        try:
            ret, this, *a = struct.unpack(f"<{2 + m.n}I", bytes(uc.mem_read(esp, 8 + 4 * m.n)))
        except UcError:
            raise Unsupported("stack frame is not mapped")
        name = f"{iface}::{m.name}"
        if m.unsupported:
            raise Unsupported(f"{name}: {m.unsupported}")
        if not this:
            raise Unsupported(f"{name} on a NULL object")
        hashes = tuple(self._hash_in(a, i, size) for i, size in m.ins)
        norm = self.stubs._norm
        self.stubs.log.append((name, norm(this)) + tuple(norm(v) for v in a) + hashes)
        rng = self.rng
        if m.ret == "hr":
            ok = not (m.fails and self.failures and rng.randrange(m.rate) == 0)
            eax = S_OK if ok else rng.choice(m.fails)
        else:
            ok = True
            if m.ret == "ulong":
                eax = rng.randrange(4)
            elif m.ret == "void":
                eax = 0
            elif m.ret == "u32":
                eax = rng.choice((0, 1, 2, rng.getrandbits(32)))
            elif m.ret.startswith("u32:"):
                eax = rng.choice([int(v, 0) for v in m.ret[4:].split(",")])
            elif m.ret == "type":
                eax = RESOURCE_TYPE.get(iface, 0)
            elif m.ret == "levels":
                eax = _levels(self.state(this))
            else:
                raise AssertionError(m.ret)
        if m.eff is not None:
            override = m.eff(self, this, a, ok)
            if override is not None:
                eax = override
        uc.reg_write(X.UC_X86_REG_EAX, eax & 0xFFFFFFFF)
        uc.reg_write(X.UC_X86_REG_ECX, 0)                     # volatile registers: a fixed clobber
        uc.reg_write(X.UC_X86_REG_EDX, 0)
        uc.reg_write(X.UC_X86_REG_ESP, esp + 8 + 4 * m.n)
        uc.reg_write(X.UC_X86_REG_EIP, ret)


# DirectInput 8 and DirectSound (appended after Direct3D, so the Direct3D vtable addresses stay put)
from .com_dx import EXTRA as _DX_IFACES, SEEDED as _DX_SEEDED  # noqa: E402
IFACES.update(_DX_IFACES)
SEEDED.update(_DX_SEEDED)
NAMES[:] = list(IFACES)
