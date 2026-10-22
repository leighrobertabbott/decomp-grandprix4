"""DirectInput 8 and DirectSound models for the COM oracle (see gp4re/com.py).

Method tables follow the Windows SDK headers dinput.h / dsound.h (method order and argument
counts were extracted from DECLARE_INTERFACE_ blocks with SAL annotations stripped). The same
rules as for Direct3D apply: every call is logged with its arguments and the content of each
input structure the real method reads (respecting the "which fields are valid" flags of
IDirectInputEffect::SetParameters), results are seeded, outputs are seeded but structurally
plausible, and anything with callbacks (Enum*), private semantics or variable output layouts
ends the run as external.

QueryInterface is modelled for the IIDs below only (taken from the same headers): it returns
the persistent object of that interface for this object (a sound buffer's IDirectSound3DBuffer,
the primary buffer's IDirectSound3DListener, ...) or E_NOINTERFACE when the source object
cannot provide it; an IID outside this table still ends the run as external.
"""
from __future__ import annotations

import struct

from .com import (D3DERR_INVALIDCALL, E_OUTOFMEMORY, M, Unsupported, put_bytes, put_obj, put_u32)

E_NOINTERFACE, E_INVALIDARG, E_ACCESSDENIED = 0x80004002, 0x80070057, 0x80070005
DIERR_INPUTLOST, DIERR_NOTACQUIRED, DIERR_DEVICENOTREG = 0x8007001E, 0x8007000C, 0x80040154
DIERR_NOTEXCLUSIVEACQUIRED, DI_BUFFEROVERFLOW = 0x80040205, 0x00000001
DSERR_BUFFERLOST, DSERR_INVALIDPARAM, DSERR_ALLOCATED = 0x88780096, 0x80070057, 0x8878000A
DSERR_NODRIVER = 0x88780078


def _guid(d1, d2, d3, *b):
    return struct.pack("<IHH8B", d1, d2, d3, *b)


IIDS = {
    _guid(3305799315, 62357, 18484, 158, 246, 127, 169, 157, 229, 9, 102): "IDirectSound8",
    _guid(664468101, 18817, 4558, 165, 33, 0, 32, 175, 11, 229, 96): "IDirectSoundBuffer",
    _guid(1747297353, 29988, 19842, 146, 15, 80, 227, 106, 179, 171, 30): "IDirectSoundBuffer8",
    _guid(664468102, 18817, 4558, 165, 33, 0, 32, 175, 11, 229, 96): "IDirectSound3DBuffer",
    _guid(664468100, 18817, 4558, 165, 33, 0, 32, 175, 11, 229, 96): "IDirectSound3DListener",
    _guid(2954954627, 35277, 4560, 175, 8, 0, 160, 201, 37, 205, 22): "IDirectSoundNotify",
    _guid(3212410928, 18490, 19874, 170, 153, 93, 100, 237, 54, 151, 0): "IDirectInput8A",
    _guid(1423184000, 56341, 18483, 164, 27, 116, 143, 115, 163, 129, 121): "IDirectInputDevice8A",
    _guid(3890345920, 35026, 4560, 154, 208, 0, 160, 201, 160, 110, 53): "IDirectInputEffect",
}
# which interfaces an object of each kind hands out through QueryInterface
QI_FROM = {
    "IDirectSoundBuffer": ("IDirectSoundBuffer", "IDirectSoundBuffer8", "IDirectSound3DBuffer",
                           "IDirectSound3DListener", "IDirectSoundNotify"),
    "IDirectSoundBuffer8": ("IDirectSoundBuffer", "IDirectSoundBuffer8", "IDirectSound3DBuffer", "IDirectSoundNotify"),
    "IDirectSound8": ("IDirectSound8",),
    "IDirectSound3DBuffer": ("IDirectSound3DBuffer", "IDirectSoundBuffer", "IDirectSoundBuffer8"),
    "IDirectSound3DListener": ("IDirectSound3DListener", "IDirectSoundBuffer"),
    "IDirectSoundNotify": ("IDirectSoundNotify",),
    "IDirectInput8A": ("IDirectInput8A",),
    "IDirectInputDevice8A": ("IDirectInputDevice8A",),
    "IDirectInputEffect": ("IDirectInputEffect",),
}


def query_interface(model, this, a, ok):
    if not a[1]:
        return E_INVALIDARG
    want = IIDS.get(model.read(a[0], 16))
    if want is None:
        raise Unsupported("QueryInterface for an IID the model does not know")
    have = model.state(this)["iface"]
    if want not in QI_FROM.get(have, ()):
        model.put(a[1], bytes(4))
        return E_NOINTERFACE
    obj = this if want == have else model.child(this, ("qi", want), want)
    if obj != this:            # the new interface shares the buffer's state (size, format, lock memory)
        st = model.state(obj)
        for k, v in model.state(this).items():
            if k != "iface":
                st.setdefault(k, v)
        st.setdefault("base", model.state(this).get("base", this))
    model.put(a[1], struct.pack("<I", obj))
    return 0


QI = M("QueryInterface", 2, ins=((0, 16),), eff=query_interface, fails=())
IUNKNOWN = [QI, M("AddRef", 0, ret="ulong"), M("Release", 0, ret="ulong")]
ENUM = "enumeration callbacks are not modelled"


def put_pair(gen):
    """Two out pointers (a getter returning a pair): each non-null one gets gen()."""
    def eff(model, this, a, ok):
        if ok:
            for p in a[:2]:
                if p:
                    model.put(p, gen(model))
    return eff


def put_sized(i, gen, minimum=8, maximum=0x1000):
    """Out structure whose first dword (dwSize) the caller set: keep it, seed the rest."""
    def eff(model, this, a, ok):
        if not ok or not a[i]:
            return None
        size = struct.unpack("<I", model.read(a[i], 4))[0]
        if not minimum <= size <= maximum:
            return E_INVALIDARG
        body = gen(model, this, a, size)
        model.put(a[i] + 4, body[: size - 4].ljust(size - 4, b"\0"))
    return eff


def _ascii(text: str, n: int) -> bytes:
    return text.encode()[: n - 1].ljust(n, b"\0")


def _unptr(data: bytes, *offsets) -> bytes:
    """Replace pointer fields inside a hashed structure by a presence marker: the pointees are
    hashed on their own, and a pointer's value (often a stack address) differs between the
    original's and the reconstruction's frames even when the calls are identical."""
    b = bytearray(data)
    for off in offsets:
        if off + 4 <= len(b):
            b[off: off + 4] = (b"\xff\xff\xff\xff" if any(b[off: off + 4]) else bytes(4))
    return bytes(b)


# ---------------------------------------------------------------- DirectInput
def dieffect_size(flags_from):
    """Bytes of a DIEFFECT the method reads: header plus the arrays its dwFlags/DIEP flags select."""
    def size(a, read):
        p = a[flags_from[0]]
        params = a[flags_from[1]] if flags_from[1] is not None else 0xFFFFFFFF
        head = read(p, 56)
        dw_size, _, _, _, _, _, _, axes, p_axes, p_dir, p_env, cb_ts, p_ts, _ = struct.unpack("<14I", head)
        if dw_size not in (52, 56):
            raise Unsupported(f"DIEFFECT dwSize {dw_size}")
        if axes > 8 or cb_ts > 0x400:
            raise Unsupported("DIEFFECT with more than 8 axes or large type-specific parameters")
        parts = [_unptr(head[:dw_size], 32, 36, 40, 48)]   # rgdwAxes, rglDirection, lpEnvelope, lpvTypeSpecificParams
        if params & 0x20 and axes:        # DIEP_AXES
            parts.append(read(p_axes, 4 * axes))
        if params & 0x40 and axes:        # DIEP_DIRECTION
            parts.append(read(p_dir, 4 * axes))
        if params & 0x80 and p_env:       # DIEP_ENVELOPE
            parts.append(read(p_env, 20))
        if params & 0x100 and cb_ts:      # DIEP_TYPESPECIFICPARAMS
            parts.append(read(p_ts, cb_ts))
        return ("bytes", b"".join(parts))
    return size


def didataformat_size(a, read):
    head = read(a[0], 24)
    _, obj_size, _, _, n_objs, p_objs = struct.unpack("<6I", head)
    if obj_size != 16 or n_objs > 512:
        raise Unsupported("DIDATAFORMAT with unexpected object size or count")
    objs = read(p_objs, 16 * n_objs) if n_objs else b""
    guids = b"".join(read(struct.unpack_from("<I", objs, 16 * k)[0], 16) if struct.unpack_from("<I", objs, 16 * k)[0]
                     else b"null" for k in range(n_objs))
    objs = _unptr(objs, *range(0, 16 * n_objs, 16))      # each object's pguid (hashed below)
    return ("bytes", _unptr(head, 20) + objs + guids)


def diprop_size(a, read):
    return ("bytes", read(a[1], max(16, min(struct.unpack("<I", read(a[1], 4))[0], 0x100))))


def guid_or_id(i):
    """A REFGUID that may be a MAKEDIPROP integer (property ids below 0x10000 are not pointers)."""
    def size(a, read):
        return ("bytes", struct.pack("<I", a[i])) if a[i] < 0x10000 else ("bytes", read(a[i], 16))
    return size


def get_property(model, this, a, ok):
    if not ok or not a[1]:
        return None
    size, header = struct.unpack("<2I", model.read(a[1], 8))
    r = model.rng
    if size == 20:      # DIPROPDWORD
        model.put(a[1] + 16, struct.pack("<I", r.choice((0, 1, 16, 32, 100, 10000))))
    elif size == 24:    # DIPROPRANGE
        lo = r.choice((-10000, -1000, 0, -32768))
        model.put(a[1] + 16, struct.pack("<2i", lo, r.choice((1000, 10000, 32767, 65535))))
    else:
        raise Unsupported(f"GetProperty with a {size}-byte property structure")


def device_state(model, this, a, ok):
    if not ok or not a[1]:
        return None
    n = a[0]
    if n > 0x1000:
        return E_INVALIDARG
    model.put(a[1], model.rng.randbytes(n))


def device_data(model, this, a, ok):
    if not a[2]:
        return E_INVALIDARG
    want = struct.unpack("<I", model.read(a[2], 4))[0]
    if not ok:
        return None
    n = min(want, model.rng.choice((0, 0, 1, 2, 4)))
    if a[1] and n:
        if a[0] not in (16, 20) or n * a[0] > 0x1000:
            raise Unsupported("GetDeviceData with an unexpected object size")
        model.put(a[1], model.rng.randbytes(n * a[0]))
    model.put(a[2], struct.pack("<I", n))
    return DI_BUFFEROVERFLOW if model.rng.randrange(16) == 0 else None


def dev_caps(model, this, a, size):
    r = model.rng
    return struct.pack("<10I", r.getrandbits(32), r.choice((0x10114, 0x10115, 0x10116, 0x10112, 0x10113)),
                       r.randint(0, 6), r.randint(0, 32), r.randint(0, 4), r.choice((0, 1000, 10000)),
                       r.choice((0, 1000)), r.getrandbits(16), r.getrandbits(16), r.getrandbits(16))


def dev_instance(model, this, a, size):
    r = model.rng
    return (r.randbytes(32) + struct.pack("<I", r.choice((0x10114, 0x10115, 0x10116, 0x10112, 0x10113)))
            + _ascii(f"Game controller {r.randrange(8)}", 260) + _ascii(f"Controller {r.randrange(8)}", 260)
            + r.randbytes(16) + struct.pack("<2H", 1, r.choice((4, 5))))


def obj_instance(model, this, a, size):
    r = model.rng
    return (r.randbytes(16) + struct.pack("<3I", r.choice((0, 4, 8, 12, 0x30, 0x34)), r.getrandbits(32), r.getrandbits(32))
            + _ascii(f"Axis {r.randrange(8)}", 260) + struct.pack("<2I4HIHH", r.choice((0, 10000)), r.choice((0, 1, 100)),
                                                                  0, 0, 1, r.randrange(0x40), 0, 0, 0))


DIRECTINPUT8 = IUNKNOWN + [
    M("CreateDevice", 3, ins=((0, 16),), eff=put_obj(1, "IDirectInputDevice8A"),
      fails=(DIERR_DEVICENOTREG, E_OUTOFMEMORY)),
    M("EnumDevices", 4, unsupported=ENUM),
    M("GetDeviceStatus", 1, ins=((0, 16),), fails=(DIERR_DEVICENOTREG,), rate=3),
    M("RunControlPanel", 2), M("Initialize", 2),
    M("FindDevice", 3, unsupported="FindDevice results are not modelled"),
    M("EnumDevicesBySemantics", 5, unsupported=ENUM),
    M("ConfigureDevices", 4, unsupported="the configuration UI is not modelled")]

DIDEVICE8 = IUNKNOWN + [
    M("GetCapabilities", 1, eff=put_sized(0, dev_caps, 44, 44)),
    M("EnumObjects", 3, unsupported=ENUM),
    M("GetProperty", 2, ins=((0, guid_or_id(0)), (1, diprop_size)), eff=get_property),
    M("SetProperty", 2, ins=((0, guid_or_id(0)), (1, diprop_size))),
    M("Acquire", 0, fails=(E_ACCESSDENIED, DIERR_INPUTLOST)),
    M("Unacquire", 0, fails=()),
    M("GetDeviceState", 2, eff=device_state, fails=(DIERR_INPUTLOST, DIERR_NOTACQUIRED)),
    M("GetDeviceData", 4, eff=device_data, fails=(DIERR_INPUTLOST, DIERR_NOTACQUIRED)),
    M("SetDataFormat", 1, ins=((0, didataformat_size),)),
    M("SetEventNotification", 1),
    M("SetCooperativeLevel", 2, fails=(E_INVALIDARG,)),
    M("GetObjectInfo", 3, eff=put_sized(0, obj_instance, 0x13c, 0x13c)),
    M("GetDeviceInfo", 1, eff=put_sized(0, dev_instance, 0x244, 0x244)),
    M("RunControlPanel", 2), M("Initialize", 3, ins=((2, 16),)),
    M("CreateEffect", 4, ins=((0, 16), (1, dieffect_size((1, None)))), eff=put_obj(2, "IDirectInputEffect"),
      fails=(E_INVALIDARG, DIERR_NOTEXCLUSIVEACQUIRED, E_OUTOFMEMORY)),
    M("EnumEffects", 3, unsupported=ENUM),
    M("GetEffectInfo", 2, unsupported="GetEffectInfo results are not modelled"),
    M("GetForceFeedbackState", 1, eff=put_u32(0, lambda m, t, a: m.rng.getrandbits(32))),
    M("SendForceFeedbackCommand", 1),
    M("EnumCreatedEffectObjects", 3, unsupported=ENUM),
    M("Escape", 1, unsupported="Escape is not modelled"),
    M("Poll", 0, fails=(DIERR_INPUTLOST, DIERR_NOTACQUIRED)),
    M("SendDeviceData", 4, unsupported="SendDeviceData is not modelled"),
    M("EnumEffectsInFile", 4, unsupported=ENUM),
    M("WriteEffectToFile", 4, unsupported="effect files are not modelled"),
    M("BuildActionMap", 3, unsupported="action maps are not modelled"),
    M("SetActionMap", 3, unsupported="action maps are not modelled"),
    M("GetImageInfo", 1, unsupported="device images are not modelled")]

DIEFFECT = IUNKNOWN + [
    M("Initialize", 3, ins=((2, 16),)),
    M("GetEffectGuid", 1, eff=put_bytes(0, lambda m, t, a: m.rng.randbytes(16))),
    M("GetParameters", 2, unsupported="GetParameters results are not modelled"),
    M("SetParameters", 2, ins=((0, dieffect_size((0, 1))),), fails=(E_INVALIDARG, DIERR_INPUTLOST, DIERR_NOTEXCLUSIVEACQUIRED)),
    M("Start", 2, fails=(DIERR_INPUTLOST, DIERR_NOTEXCLUSIVEACQUIRED)),
    M("Stop", 0),
    M("GetEffectStatus", 1, eff=put_u32(0, lambda m, t, a: m.rng.randrange(2))),
    M("Download", 0, fails=(DIERR_INPUTLOST, DIERR_NOTEXCLUSIVEACQUIRED)),
    M("Unload", 0),
    M("Escape", 1, unsupported="Escape is not modelled")]


# ---------------------------------------------------------------- DirectSound
def wfx_size(i, optional=True):
    def size(a, read):
        p = a[i]
        head = read(p, 18)
        tag, _, _, _, _, _, cb = struct.unpack("<HHIIHHH", head)
        return ("bytes", head + (read(p + 18, cb) if tag != 1 and 0 < cb <= 0x100 else b""))
    return size


def dsbufferdesc_size(a, read):
    head = read(a[0], 20)
    size, flags, nbytes, _, p_wfx = struct.unpack("<5I", head)
    if size not in (20, 36):
        raise Unsupported(f"DSBUFFERDESC dwSize {size}")
    out = _unptr(read(a[0], size), 16)                    # lpwfxFormat (hashed below)
    if p_wfx:
        out += wfx_size(0)([p_wfx], read)[1]
    return ("bytes", out)


def create_buffer(model, this, a, ok):
    if not a[1]:
        return E_INVALIDARG
    if not ok:
        model.put(a[1], bytes(4))
        return None
    size, flags, nbytes, _, p_wfx = struct.unpack("<5I", model.read(a[0], 20))
    if not flags & 1 and not 4 <= nbytes <= 0x1000000:      # DSBCAPS_PRIMARYBUFFER has no size
        return DSERR_INVALIDPARAM
    wfx = model.read(p_wfx, 18) if p_wfx else struct.pack("<HHIIHHH", 1, 2, 22050, 88200, 4, 16, 0)
    obj = model.new("IDirectSoundBuffer", flags=flags, length=nbytes if not flags & 1 else 0x4000, wfx=wfx,
                    primary=bool(flags & 1))
    model.put(a[1], struct.pack("<I", obj))


def duplicate_buffer(model, this, a, ok):
    if not a[1]:
        return E_INVALIDARG
    if not ok:
        model.put(a[1], bytes(4))
        return None
    src = model.state(a[0])
    model.put(a[1], struct.pack("<I", model.new("IDirectSoundBuffer", **{k: v for k, v in src.items() if k != "iface"})))


def ds_caps(model, this, a, size):
    r = model.rng
    return struct.pack("<23I", *[r.getrandbits(32) if k < 1 else r.choice((0, 1, 8, 32, 64, 22050, 44100, 100000))
                                 for k in range(23)])


def buffer_caps(model, this, a, ok):
    if not ok or not a[0]:
        return None
    st = model.state(this)
    size = struct.unpack("<I", model.read(a[0], 4))[0]
    if size != 20:
        return DSERR_INVALIDPARAM
    model.put(a[0] + 4, struct.pack("<4I", st.get("flags", 0), st.get("length", 0), 0, model.rng.randrange(100)))


def positions(model, this, a, ok):
    if not ok:
        return None
    n = max(1, model.state(this).get("length", 0x4000))
    for p in (a[0], a[1]):
        if p:
            model.put(p, struct.pack("<I", model.rng.randrange(n)))


def get_format(model, this, a, ok):
    wfx = model.state(this).get("wfx", struct.pack("<HHIIHHH", 1, 2, 22050, 88200, 4, 16, 0))
    if a[0]:
        if a[1] < len(wfx):
            return DSERR_INVALIDPARAM
        if ok:
            model.put(a[0], wfx)
    if a[2] and ok:
        model.put(a[2], struct.pack("<I", len(wfx)))


def lock_ring(model, this, a, ok):
    st = model.state(this)
    n = st.get("length", 0)
    offset, nbytes, flags = a[0], a[1], a[6]
    if flags & 1:                                    # DSBLOCK_FROMWRITECURSOR
        offset = model.rng.randrange(max(1, n))
    if flags & 2:                                    # DSBLOCK_ENTIREBUFFER
        nbytes = n
    if not a[2] or not a[3] or offset >= max(1, n) or nbytes > n or nbytes == 0:
        return DSERR_INVALIDPARAM
    if not ok:
        return None
    base = st.get("base", this)
    buf, _ = model.lockbuf(base, "ring", n, 1)
    first = min(nbytes, n - offset)
    model.put(a[2], struct.pack("<I", buf + offset))
    model.put(a[3], struct.pack("<I", first))
    if a[4]:
        model.put(a[4], struct.pack("<I", buf if nbytes > first else 0))
    if a[5]:
        model.put(a[5], struct.pack("<I", nbytes - first))


def put_i32_range(i, lo, hi):
    return put_u32(i, lambda m, t, a: m.rng.randint(lo, hi))


def put_floats(i, n, lo=-100.0, hi=100.0):
    return put_bytes(i, lambda m, t, a: b"".join(struct.pack("<f", m.rng.uniform(lo, hi)) for _ in range(n)))


DS_FAILS = (DSERR_INVALIDPARAM,)
DSOUND8 = IUNKNOWN + [
    M("CreateSoundBuffer", 3, ins=((0, dsbufferdesc_size),), eff=create_buffer,
      fails=(DSERR_INVALIDPARAM, DSERR_ALLOCATED, E_OUTOFMEMORY)),
    M("GetCaps", 1, eff=put_sized(0, ds_caps, 96, 96)),
    M("DuplicateSoundBuffer", 2, eff=duplicate_buffer, fails=(DSERR_ALLOCATED, E_OUTOFMEMORY)),
    M("SetCooperativeLevel", 2, fails=DS_FAILS),
    M("Compact", 0), M("GetSpeakerConfig", 1, eff=put_u32(0, lambda m, t, a: m.rng.choice((1, 2, 3, 4, 5, 6)))),
    M("SetSpeakerConfig", 1), M("Initialize", 1, ins=((0, 16),), fails=(DSERR_NODRIVER,)),
    M("VerifyCertification", 1, eff=put_u32(0, lambda m, t, a: m.rng.randrange(2)))]

DSBUFFER = IUNKNOWN + [
    M("GetCaps", 1, eff=buffer_caps),
    M("GetCurrentPosition", 2, eff=positions, fails=(DSERR_INVALIDPARAM, DSERR_BUFFERLOST)),
    M("GetFormat", 3, eff=get_format),
    M("GetVolume", 1, eff=put_i32_range(0, -10000, 0)),
    M("GetPan", 1, eff=put_i32_range(0, -10000, 10000)),
    M("GetFrequency", 1, eff=put_i32_range(0, 100, 100000)),
    M("GetStatus", 1, eff=put_u32(0, lambda m, t, a: m.rng.choice((0, 1, 5, 2, 0x10)))),
    M("Initialize", 2, unsupported="IDirectSoundBuffer::Initialize is not modelled"),
    M("Lock", 7, eff=lock_ring, fails=(DSERR_BUFFERLOST, DSERR_INVALIDPARAM)),
    M("Play", 3, fails=(DSERR_BUFFERLOST, DSERR_INVALIDPARAM)),
    M("SetCurrentPosition", 1), M("SetFormat", 1, ins=((0, wfx_size(0)),)),
    M("SetVolume", 1), M("SetPan", 1), M("SetFrequency", 1),
    M("Stop", 0), M("Unlock", 4), M("Restore", 0, fails=(DSERR_BUFFERLOST,))]
DSBUFFER8 = DSBUFFER + [M("SetFX", 3, unsupported="effects chains are not modelled"),
                        M("AcquireResources", 3, unsupported="voice management is not modelled"),
                        M("GetObjectInPath", 4, unsupported="GetObjectInPath is not modelled")]

DS3DBUFFER = IUNKNOWN + [
    M("GetAllParameters", 1, eff=put_sized(0, lambda m, t, a, s: m.rng.randbytes(60), 64, 64)),
    M("GetConeAngles", 2, eff=put_pair(lambda m: struct.pack("<I", m.rng.randrange(361)))),
    M("GetConeOrientation", 1, eff=put_floats(0, 3, -1, 1)),
    M("GetConeOutsideVolume", 1, eff=put_i32_range(0, -10000, 0)),
    M("GetMaxDistance", 1, eff=put_floats(0, 1, 1, 1e9)),
    M("GetMinDistance", 1, eff=put_floats(0, 1, 0, 100)),
    M("GetMode", 1, eff=put_u32(0, lambda m, t, a: m.rng.randrange(3))),
    M("GetPosition", 1, eff=put_floats(0, 3)),
    M("GetVelocity", 1, eff=put_floats(0, 3)),
    M("SetAllParameters", 2, ins=((0, 64),)),
    M("SetConeAngles", 3), M("SetConeOrientation", 4), M("SetConeOutsideVolume", 2),
    M("SetMaxDistance", 2), M("SetMinDistance", 2), M("SetMode", 2), M("SetPosition", 4), M("SetVelocity", 4)]

DS3DLISTENER = IUNKNOWN + [
    M("GetAllParameters", 1, eff=put_sized(0, lambda m, t, a, s: m.rng.randbytes(60), 64, 64)),
    M("GetDistanceFactor", 1, eff=put_floats(0, 1, 0.01, 10)),
    M("GetDopplerFactor", 1, eff=put_floats(0, 1, 0, 10)),
    M("GetOrientation", 2, eff=put_pair(lambda m: struct.pack("<3f", *(m.rng.uniform(-1, 1) for _ in range(3))))),
    M("GetPosition", 1, eff=put_floats(0, 3)),
    M("GetRolloffFactor", 1, eff=put_floats(0, 1, 0, 10)),
    M("GetVelocity", 1, eff=put_floats(0, 3)),
    M("SetAllParameters", 2, ins=((0, 64),)),
    M("SetDistanceFactor", 2), M("SetDopplerFactor", 2), M("SetOrientation", 7), M("SetPosition", 4),
    M("SetRolloffFactor", 2), M("SetVelocity", 4), M("CommitDeferredSettings", 0)]

DSNOTIFY = IUNKNOWN + [M("SetNotificationPositions", 2, ins=((1, lambda a: 8 * a[0]),), fails=DS_FAILS)]

EXTRA = {
    "IDirectInput8A": DIRECTINPUT8,
    "IDirectInputDevice8A": DIDEVICE8,
    "IDirectInputEffect": DIEFFECT,
    "IDirectSound8": DSOUND8,
    "IDirectSoundBuffer": DSBUFFER,
    "IDirectSoundBuffer8": DSBUFFER8,
    "IDirectSound3DBuffer": DS3DBUFFER,
    "IDirectSound3DListener": DS3DLISTENER,
    "IDirectSoundNotify": DSNOTIFY,
}
assert [len(v) for v in EXTRA.values()] == [11, 32, 13, 12, 21, 24, 21, 18, 4]


def _buffer_state(rng):
    rate = rng.choice((11025, 22050, 44100))
    return {"flags": rng.choice((0x80, 0xE0, 0x10, 0x18)), "length": rng.choice((0x1000, 0x4000, 0x10000, 0x40000)),
            "wfx": struct.pack("<HHIIHHH", 1, 2, rate, rate * 4, 4, 16, 0), "primary": False}


SEEDED = {"IDirectSoundBuffer": _buffer_state, "IDirectSoundBuffer8": _buffer_state,
          "IDirectSound3DBuffer": _buffer_state}
