"""Deterministic models of a few Win32 imports for the differential oracle.

Without them every import call ends an emulated run as "external" (an invalid case), so
any function that allocates, frees, logs or reads a clock cannot be checked at T1. The
modelled APIs have no effect outside memory the oracle already compares:

  kernel32  GetProcessHeap, HeapAlloc, HeapFree, HeapReAlloc, HeapSize,
            OutputDebugStringA, GetTickCount, QueryPerformanceCounter,
            QueryPerformanceFrequency, Sleep,
            GlobalAlloc (fixed memory), GlobalFree, GlobalHandle, GlobalMemoryStatus,
            GetLocalTime, GetSystemTime,
            CreateFileA, ReadFile, WriteFile, SetFilePointer, GetFileSize, CloseHandle,
            FindFirstFileA, FindNextFileA, FindClose, DeleteFileA, GetFileAttributesA,
            CreateDirectoryA, SetCurrentDirectoryA, GetCurrentDirectoryA, GetLogicalDrives,
            GetDriveTypeA
  user32    GetKeyNameTextA, LoadCursorA, SetCursor, ShowCursor, MapVirtualKeyA, MessageBoxA, MessageBoxExA, PostMessageA (queued, no effect here)
  gdi32     SelectObject, SetTextColor, SetBkColor, GetTextExtentPoint32A
  winmm     timeGetTime
  CRT support (single-threaded run): Initialize/Enter/Leave/DeleteCriticalSection (XP field
            semantics), InterlockedIncrement/Decrement/Exchange, GetCurrentThreadId/ProcessId,
            TlsAlloc/GetValue/SetValue/Free (per-run slots), GetLastError/SetLastError,
            VirtualAlloc/VirtualFree (bump allocator in a mapped region; the CRT small-block
            heap), MultiByteToWideChar/WideCharToMultiByte (code page 1252), GetStdHandle,
            GetModuleFileNameA
  d3d8      Direct3DCreate8 (the IDirect3D8 it returns and every object made from it: gp4re/com.py)
  dinput8   DirectInput8Create (IDirectInput8A and its devices/effects: gp4re/com_dx.py)
  dsound    #11 DirectSoundCreate8 (IDirectSound8 and its buffers: gp4re/com_dx.py)

Files are a seeded fake file system private to the run: CreateFileA opens a new file of
seeded size and contents (or fails, sometimes), reads/seeks/sizes act on it, writes are
logged by content. A handle in 0x3000..0x3ffc that the run did not open is a file a caller
opened earlier (layouts store such values in handle globals): it gets seeded size and
contents on first use. Any other handle the run did not open is an invalid handle (the
call fails the way Windows fails it). GDI, keyboard and message-box results are seeded values.

Each run starts from the same stub state (a per-case seed shared by the original and the
reconstruction runs of a case), so equal call sequences see equal results: the heap is a
bump allocator in its own mapped region (blocks without HEAP_ZERO_MEMORY are filled with
seeded bytes), clocks advance by seeded steps. Every call is logged with its arguments
(stack addresses normalised to "stack", strings by content); the oracle requires the
original's and the reconstruction's logs to be identical, so a reconstruction that frees,
allocates or logs differently fails even when the stub's results would hide it.
Anything a model cannot represent faithfully (re-allocating a block this run did not
allocate, writing a result through an unmapped pointer) ends the run as "external".
"""
from __future__ import annotations

import hashlib
import random
import struct

from unicorn import UcError
from unicorn import x86_const as X

HEAP_BASE, HEAP_SIZE = 0x30000000, 0x01000000
VALLOC_BASE, VALLOC_SIZE = 0x31000000, 0x01000000      # VirtualAlloc'd pages (CRT small-block heap)
THREAD_ID, PROCESS_ID = 0x00000F00, 0x00000E00
MODULE_PATH = "C:\\GP4\\GP4.exe"
HEAP_HANDLE = 0x00150000
MAX_BLOCK = 0x00400000
QPC_FREQUENCY = 3579545
HEAP_ZERO_MEMORY = 0x8
GMEM_MOVEABLE, GMEM_ZEROINIT = 0x2, 0x40
INVALID_HANDLE = 0xFFFFFFFF
FILE_HANDLE_BASE, FIND_HANDLE_BASE = 0x00001000, 0x00002000
PREOPEN_LO, PREOPEN_HI = 0x00003000, 0x00004000   # handles a caller opened before this run
MAX_IO = 0x00400000
CREATE_NEW, CREATE_ALWAYS, TRUNCATE_EXISTING = 1, 2, 5


class _File:
    def __init__(self, data: bytes):
        self.data, self.pos = bytearray(data), 0


from .com import ComModel, Unsupported   # noqa: E402  (Unsupported: the run is external)


class ImportStubs:
    def __init__(self, uc, mark_dirty, stack_range: tuple[int, int]):
        self.uc = uc
        self.mark_dirty = mark_dirty          # (addr, size) -> None: pages the stub wrote
        self.stack_lo, self.stack_hi = stack_range
        self.seed = 0
        # memory the oracle's init calls allocated (CRT onexit table, ...) survives every reset
        self.floor, self.floor_blocks, self.floor_valloc = HEAP_BASE, {}, VALLOC_BASE
        self.com = ComModel(self)
        self.reset()

    # ---------------------------------------------------------------- state
    def reset(self, seed: int | None = None) -> None:
        if seed is not None:
            self.seed = seed
        self.rng = random.Random((self.seed * 0x9E3779B1 ^ 0x5354554253) & 0xFFFFFFFFFFFF)
        self.heap_next = self.floor
        self.blocks: dict[int, int] = dict(self.floor_blocks)
        self.log: list[tuple] = []
        self.clock = 0x01000000 + self.rng.randrange(1 << 24)
        self.qpc = (1 << 32) + self.rng.randrange(1 << 40)
        self.files: dict[int, _File] = {}
        self.finds: dict[int, list[str]] = {}
        self.next_file, self.next_find = FILE_HANDLE_BASE, FIND_HANDLE_BASE
        self.closed: set[int] = set()
        self.com.reset()
        self.last_error = 0
        self.tls: dict[int, int] = {}
        self.tls_next = 1
        self.valloc_next = self.floor_valloc

    def handles(self, name: str) -> bool:
        return name in STUBS

    # ---------------------------------------------------------------- helpers
    def _norm(self, v: int):
        return "stack" if self.stack_lo <= v < self.stack_hi else v

    def _write(self, addr: int, data: bytes) -> None:
        try:
            self.uc.mem_write(addr, data)
        except UcError:
            raise Unsupported(f"result pointer {addr:#010x} is not mapped")
        self.mark_dirty(addr, len(data))

    def _cstring(self, addr: int, limit: int = 4096) -> str:
        out = bytearray()
        try:
            while len(out) < limit:
                chunk = bytes(self.uc.mem_read(addr + len(out), 64))
                end = chunk.find(0)
                if end >= 0:
                    out += chunk[:end]
                    return out.decode("latin1")
                out += chunk
        except UcError:
            raise Unsupported(f"string at {addr:#010x} is not mapped")
        raise Unsupported(f"string at {addr:#010x} is not NUL-terminated within {limit} bytes")

    def _alloc(self, size: int, zero: bool) -> int:
        if size > MAX_BLOCK or self.heap_next + size > HEAP_BASE + HEAP_SIZE:
            return 0
        p = self.heap_next
        self.heap_next = (p + max(size, 1) + 0x17) & ~0xF     # 8-byte gap, 16-byte aligned blocks
        self.blocks[p] = size
        if size:
            self._write(p, bytes(size) if zero else self.rng.randbytes(size))
        return p

    # ---------------------------------------------------------------- models
    def get_process_heap(self):
        return HEAP_HANDLE, ()

    def heap_alloc(self, heap, flags, size):
        return self._alloc(size, bool(flags & HEAP_ZERO_MEMORY)), (heap, flags, size)

    def heap_free(self, heap, flags, p):
        self.blocks.pop(p, None)
        return 1, (heap, flags, self._norm(p))

    def heap_realloc(self, heap, flags, p, size):
        if p not in self.blocks:
            raise Unsupported(f"HeapReAlloc of {p:#010x}, a block this run did not allocate")
        old = self.blocks.pop(p)
        q = self._alloc(size, bool(flags & HEAP_ZERO_MEMORY))
        if q:
            keep = min(old, size)
            self._write(q, bytes(self.uc.mem_read(p, keep)))
        return q, (heap, flags, p, size)

    def heap_size(self, heap, flags, p):
        return self.blocks.get(p, 0xFFFFFFFF), (heap, flags, self._norm(p))

    def output_debug_string(self, s):
        return 0, (self._cstring(s),)

    def tick(self):
        self.clock = (self.clock + self.rng.randint(1, 16)) & 0xFFFFFFFF
        return self.clock, ()

    def qpc_counter(self, p):
        self.qpc += self.rng.randint(1, 50000)
        self._write(p, struct.pack("<Q", self.qpc & 0xFFFFFFFFFFFFFFFF))
        return 1, (self._norm(p),)

    def qpc_frequency(self, p):
        self._write(p, struct.pack("<Q", QPC_FREQUENCY))
        return 1, (self._norm(p),)

    def sleep(self, ms):
        return 0, (ms,)

    # global memory (GP4 imports no GlobalLock: blocks are fixed, handle == pointer)
    def global_alloc(self, flags, size):
        if flags & GMEM_MOVEABLE:
            raise Unsupported("GlobalAlloc(GMEM_MOVEABLE) handles are not modelled")
        return self._alloc(size, bool(flags & GMEM_ZEROINIT)), (flags, size)

    def global_free(self, h):
        self.blocks.pop(h, None)
        return 0, (self._norm(h),)

    def global_handle(self, p):
        return p, (self._norm(p),)

    def global_memory_status(self, p):
        total = self.rng.choice([0x01000000, 0x02000000, 0x03000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000])
        page = total * 2
        self._write(p, struct.pack("<8I", 32, self.rng.randrange(101), total, self.rng.randrange(total + 1),
                                   page, self.rng.randrange(page + 1), 0x7FFE0000, self.rng.randrange(0x7FFE0001)))
        return 0, (self._norm(p),)

    def local_time(self, p):
        r = self.rng
        self._write(p, struct.pack("<8H", r.randrange(1980, 2040), r.randrange(1, 13), r.randrange(7), r.randrange(1, 29),
                                   r.randrange(24), r.randrange(60), r.randrange(60), r.randrange(1000)))
        return 0, (self._norm(p),)

    # fake file system
    def _file(self, h) -> "_File | None":
        f = self.files.get(h)
        if f is None and PREOPEN_LO <= h < PREOPEN_HI and h % 4 == 0 and h not in self.closed:
            f = self.files[h] = _File(self._new_file_data(3))
        return f
    def _new_file_data(self, creation) -> bytes:
        if creation in (CREATE_NEW, CREATE_ALWAYS, TRUNCATE_EXISTING):
            return b""
        r = self.rng
        size = r.choice([0, r.randrange(1, 0x40), r.randrange(0x40, 0x1000), r.randrange(0x1000, 0x10000)])
        return r.randbytes(size)

    def create_file(self, name, access, share, sec, creation, flags, template):
        logged = (self._cstring(name), access, share, self._norm(sec), creation, flags, template)
        if self.rng.randrange(5) == 0:
            self.last_error = 2                      # ERROR_FILE_NOT_FOUND
            return INVALID_HANDLE, logged
        h, self.next_file = self.next_file, self.next_file + 4
        self.files[h] = _File(self._new_file_data(creation))
        return h, logged

    def read_file(self, h, buf, n, p_read, ovl):
        logged = (h, self._norm(buf), n, self._norm(p_read), ovl)
        f = self._file(h)
        if ovl:
            raise Unsupported("overlapped ReadFile is not modelled")
        if f is None:
            if p_read:
                self._write(p_read, bytes(4))
            return 0, logged
        if n > MAX_IO:
            raise Unsupported(f"ReadFile of {n:#x} bytes")
        chunk = bytes(f.data[f.pos:f.pos + n])
        f.pos += len(chunk)
        if chunk:
            self._write(buf, chunk)
        if p_read:
            self._write(p_read, struct.pack("<I", len(chunk)))
        return 1, logged

    def write_file(self, h, buf, n, p_written, ovl):
        if ovl:
            raise Unsupported("overlapped WriteFile is not modelled")
        if n > MAX_IO:
            raise Unsupported(f"WriteFile of {n:#x} bytes")
        try:
            data = bytes(self.uc.mem_read(buf, n)) if n else b""
        except UcError:
            raise Unsupported(f"WriteFile source {buf:#010x} is not mapped")
        logged = (h, n, hashlib.sha1(data).hexdigest(), self._norm(p_written))
        f = self._file(h)
        if f is None:
            if p_written:
                self._write(p_written, bytes(4))
            return 0, logged
        end = f.pos + n
        if end > len(f.data):
            f.data.extend(bytes(end - len(f.data)))
        f.data[f.pos:end] = data
        f.pos = end
        if p_written:
            self._write(p_written, struct.pack("<I", n))
        return 1, logged

    def set_file_pointer(self, h, dist, p_high, method):
        logged = (h, dist, self._norm(p_high), method)
        f = self._file(h)
        if f is None:
            return INVALID_HANDLE, logged
        high = 0xFFFFFFFF if dist & 0x80000000 else 0
        if p_high:
            try:
                high = struct.unpack("<I", bytes(self.uc.mem_read(p_high, 4)))[0]
            except UcError:
                raise Unsupported(f"SetFilePointer high dword {p_high:#010x} is not mapped")
            logged += (high,)
        if method not in (0, 1, 2):
            raise Unsupported(f"SetFilePointer move method {method}")
        move = (high << 32 | dist) - ((1 << 64) if high & 0x80000000 else 0)
        new = (0, f.pos, len(f.data))[method] + move
        if new < 0 or new >> 32:
            return INVALID_HANDLE, logged
        f.pos = new
        if p_high:
            self._write(p_high, bytes(4))
        return new, logged

    def get_file_size(self, h, p_high):
        f = self._file(h)
        if f is None:
            return INVALID_HANDLE, (h, self._norm(p_high))
        if p_high:
            self._write(p_high, bytes(4))
        return len(f.data), (h, self._norm(p_high))

    def close_handle(self, h):
        if self._file(h) is not None:
            del self.files[h]
            self.closed.add(h)
            return 1, (h,)
        return 0, (h,)

    def _find_data(self, p, name: str) -> None:
        r = self.rng
        attrs = r.choice([0x20, 0x20, 0x10, 0x01, 0x80])
        times = r.randbytes(24)
        raw = name.encode("latin1")[:259]
        self._write(p, struct.pack("<I", attrs) + times + struct.pack("<IIII", 0, r.randrange(0x20000), 0, 0)
                    + raw + bytes(260 - len(raw)) + bytes(14) + bytes(2))

    def find_first_file(self, pattern, p):
        pat = self._cstring(pattern)
        if self.rng.randrange(5) < 2:
            return INVALID_HANDLE, (pat, self._norm(p))
        base = pat.replace("/", "\\").rsplit("\\", 1)[-1]
        names = [base.replace("*", f"G{self.rng.randrange(1000):03d}", 1).replace("*", "").replace("?", "X")
                 for _ in range(self.rng.randrange(1, 5))]
        h, self.next_find = self.next_find, self.next_find + 4
        self._find_data(p, names.pop(0))
        self.finds[h] = names
        return h, (pat, self._norm(p))

    def find_next_file(self, h, p):
        names = self.finds.get(h)
        if not names:
            return 0, (h, self._norm(p))
        self._find_data(p, names.pop(0))
        return 1, (h, self._norm(p))

    def find_close(self, h):
        return (1 if self.finds.pop(h, None) is not None else 0), (h,)

    def path_op(self, path):
        return (0 if self.rng.randrange(7) == 0 else 1), (self._cstring(path),)

    def create_directory(self, path, sec):
        return (0 if self.rng.randrange(3) == 0 else 1), (self._cstring(path), self._norm(sec))

    def file_attributes(self, path):
        return self.rng.choice([INVALID_HANDLE, 0x10, 0x20, 0x20, 0x80, 0x01]), (self._cstring(path),)

    def current_directory(self, n, buf):
        s = b"C:\\Games\\GP4"
        if n > len(s):
            self._write(buf, s + b"\0")
            return len(s), (n, self._norm(buf))
        return len(s) + 1, (n, self._norm(buf))

    # user32 / gdi32
    def map_virtual_key(self, code, kind):
        v = ((code * 2654435761 + kind * 40503) >> 9) & 0xFF      # a fixed function of the arguments
        return (0 if v < 0x20 else v & 0x7F), (code, kind)

    def message_box(self, hwnd, text, caption, kind, lang=None):
        choices = {0: [1], 1: [1, 2], 2: [3, 4, 5], 3: [6, 7, 2], 4: [6, 7], 5: [4, 2]}.get(kind & 0xF, [1, 2])
        logged = (hwnd, self._cstring(text) if text else 0, self._cstring(caption) if caption else 0, kind)
        return self.rng.choice(choices), logged + (() if lang is None else (lang,))

    def message_box_ex(self, hwnd, text, caption, kind, lang):
        return self.message_box(hwnd, text, caption, kind, lang)

    def logical_drives(self):
        return 0x4 | (self.rng.randrange(1 << 26) & ~0x3), ()          # C: always present

    def drive_type(self, root):
        return self.rng.choice([1, 2, 3, 3, 4, 5, 6]), (self._cstring(root) if root else 0,)

    def key_name(self, lparam, buf, n):
        name = f"KEY{(lparam >> 16) & 0x1ff:03X}".encode()                 # a fixed function of the scan code
        if n <= 0:
            return 0, (lparam, self._norm(buf), n)
        out = name[:n - 1]
        self._write(buf, out + bytes(1))
        return len(out), (lparam, self._norm(buf), n)

    def load_cursor(self, hinst, name):
        return 0x00010000 | (name & 0xFFFF), (hinst, name)

    def set_cursor(self, h):
        return 0x00010000 | self.rng.randrange(1, 1 << 12), (h,)

    def show_cursor(self, show):
        return self.rng.randrange(-2, 3) & 0xFFFFFFFF, (show,)

    def post_message(self, hwnd, msg, wparam, lparam):
        return 1, (hwnd, msg, wparam, lparam)

    def text_extent(self, hdc, text, n, p_size):
        if n > 0x1000:
            raise Unsupported(f"GetTextExtentPoint32A of {n:#x} characters")
        try:
            raw = bytes(self.uc.mem_read(text, n)) if n else b""
        except UcError:
            raise Unsupported(f"string at {text:#010x} is not mapped")
        h = self.rng.randrange(8, 33)
        self._write(p_size, struct.pack("<ii", sum((c % 7) + 4 for c in raw), h))   # a fixed function of the text
        return 1, (hdc, raw.decode("latin1"), n, self._norm(p_size))

    def select_object(self, hdc, obj):
        return 0x01000000 | self.rng.randrange(1, 1 << 16), (hdc, obj)

    def set_color(self, hdc, color):
        return self.rng.randrange(1 << 24), (hdc, color)

    # CRT support: the runs are single-threaded, so locks never block and interlocked ops are plain
    def _dword(self, p):
        try:
            return struct.unpack("<I", bytes(self.uc.mem_read(p, 4)))[0]
        except UcError:
            raise Unsupported(f"pointer {p:#010x} is not mapped")

    def _ints(self, p, n):
        try:
            return struct.unpack(f"<{n}i", bytes(self.uc.mem_read(p, 4 * n)))
        except UcError:
            raise Unsupported(f"pointer {p:#010x} is not mapped")

    def cs_init(self, p):                    # XP layout: DebugInfo, LockCount, RecursionCount, Owner, Sem, Spin
        self._write(p, struct.pack("<IiiIII", 0, -1, 0, 0, 0, 0))
        return 0, (self._norm(p),)

    def cs_enter(self, p):
        lock, rec = self._ints(p + 4, 2)
        self._write(p + 4, struct.pack("<iiI", lock + 1, rec + 1, THREAD_ID))
        return 0, (self._norm(p),)

    def cs_leave(self, p):
        lock, rec = self._ints(p + 4, 2)
        self._write(p + 4, struct.pack("<iiI", lock - 1, rec - 1, THREAD_ID if rec - 1 > 0 else 0))
        return 0, (self._norm(p),)

    def cs_delete(self, p):
        return 0, (self._norm(p),)

    def interlocked_inc(self, p):
        v = (self._dword(p) + 1) & 0xFFFFFFFF
        self._write(p, struct.pack("<I", v))
        return v, (self._norm(p),)

    def interlocked_dec(self, p):
        v = (self._dword(p) - 1) & 0xFFFFFFFF
        self._write(p, struct.pack("<I", v))
        return v, (self._norm(p),)

    def interlocked_xchg(self, p, v):
        old = self._dword(p)
        self._write(p, struct.pack("<I", v))
        return old, (self._norm(p), v)

    def thread_id(self):
        return THREAD_ID, ()

    def process_id(self):
        return PROCESS_ID, ()

    def tls_alloc(self):
        i, self.tls_next = self.tls_next, self.tls_next + 1
        self.tls[i] = 0
        return i, ()

    def tls_get(self, i):
        self.last_error = 0
        return self.tls.setdefault(i, 0), (i,)      # a slot allocated before this run starts empty

    def tls_set(self, i, v):
        self.tls[i] = v
        return 1, (i, self._norm(v))

    def tls_free(self, i):
        self.tls.pop(i, None)
        return 1, (i,)

    def get_last_error(self):
        return self.last_error, ()

    def set_last_error(self, v):
        self.last_error = v
        return 0, (v,)

    def virtual_alloc(self, addr, size, kind, protect):
        if addr:                              # committing pages of an earlier reservation: mapped already
            if not VALLOC_BASE <= addr < self.valloc_next:
                raise Unsupported(f"VirtualAlloc at {addr:#010x}, outside this run's reservations")
            return addr, (addr, size, kind, protect)
        size = (size + 0xFFFF) & ~0xFFFF          # allocation granularity
        if self.valloc_next + size > VALLOC_BASE + VALLOC_SIZE:
            return 0, (0, size, kind, protect)
        p = self.valloc_next
        self.valloc_next += size
        return p, (0, size, kind, protect)

    def virtual_free(self, addr, size, kind):
        return 1, (self._norm(addr), size, kind)

    def mb_to_wc(self, cp, flags, src, n, dst, cap):
        raw = (self._cstring(src).encode("latin1") + bytes(1)) if n == 0xFFFFFFFF else bytes(self.uc.mem_read(src, n))
        out = raw.decode("cp1252", errors="replace").encode("utf-16-le")
        chars = len(out) // 2
        if cap:
            if chars > cap:
                self.last_error = 122             # ERROR_INSUFFICIENT_BUFFER
                return 0, (cp, flags, raw.hex(), cap)
            self._write(dst, out)
        return chars, (cp, flags, raw.hex(), cap)

    def wc_to_mb(self, cp, flags, src, n, dst, cap, default, used):
        if n == 0xFFFFFFFF:
            buf = bytearray()
            while len(buf) < 0x10000:
                ch = bytes(self.uc.mem_read(src + len(buf), 2))
                buf += ch
                if ch == bytes(2):
                    break
            raw = bytes(buf)
        else:
            raw = bytes(self.uc.mem_read(src, 2 * n))
        out = raw.decode("utf-16-le", errors="replace").encode("cp1252", errors="replace")
        if used:
            self._write(used, struct.pack("<I", int(b"?" in out)))
        if cap:
            if len(out) > cap:
                self.last_error = 122
                return 0, (cp, flags, raw.hex(), cap)
            self._write(dst, out)
        return len(out), (cp, flags, raw.hex(), cap, self._norm(default))

    def std_handle(self, which):
        return {0xFFFFFFF6: 0x00000F10, 0xFFFFFFF5: 0x00000F14, 0xFFFFFFF4: 0x00000F18}.get(which, INVALID_HANDLE), (which,)

    def module_file_name(self, h, buf, n):
        text = MODULE_PATH.encode()
        if n:
            self._write(buf, (text + bytes(1))[:n])
        return min(len(text), max(n - 1, 0)), (h, n)

    # DirectX entry points (the objects' methods are modelled in gp4re/com.py)
    def direct3d_create(self, sdk):
        return (self.com.new("IDirect3D8") if self.rng.randrange(16) else 0), (sdk,)

    def direct_input_create(self, hinst, version, riid, pp, outer):
        from .com_dx import IIDS
        want = IIDS.get(bytes(self.uc.mem_read(riid, 16)))
        if want != "IDirectInput8A" or outer:
            raise Unsupported("DirectInput8Create for another interface or with aggregation")
        ok = self.rng.randrange(16)
        self._write(pp, struct.pack("<I", self.com.new("IDirectInput8A") if ok else 0))
        return (0 if ok else 0x80040154), (hinst, version, "IID_IDirectInput8A", self._norm(pp))

    def direct_sound_create(self, guid, pp, outer):
        if outer:
            raise Unsupported("DirectSoundCreate8 with aggregation")
        device = bytes(self.uc.mem_read(guid, 16)).hex() if guid else None
        ok = self.rng.randrange(8)
        self._write(pp, struct.pack("<I", self.com.new("IDirectSound8") if ok else 0))
        return (0 if ok else 0x88780078), (device, self._norm(pp))

    # ---------------------------------------------------------------- dispatch
    def call(self, name: str) -> None:
        """Run the model at an import trap: pop the stdcall frame and return to the caller."""
        nargs, fn = STUBS[name]
        uc = self.uc
        esp = uc.reg_read(X.UC_X86_REG_ESP)
        try:
            ret, *args = struct.unpack(f"<{1 + nargs}I", bytes(uc.mem_read(esp, 4 + 4 * nargs)))
        except UcError:
            raise Unsupported("stack frame is not mapped")
        eax, logged = fn(self, *args)
        self.log.append((name.split("!", 1)[-1],) + tuple(logged))
        uc.reg_write(X.UC_X86_REG_EAX, eax & 0xFFFFFFFF)
        uc.reg_write(X.UC_X86_REG_ECX, 0)                    # volatile registers: a fixed clobber
        uc.reg_write(X.UC_X86_REG_EDX, 0)
        uc.reg_write(X.UC_X86_REG_ESP, esp + 4 + 4 * nargs)
        uc.reg_write(X.UC_X86_REG_EIP, ret)


STUBS = {
    "kernel32.dll!GetProcessHeap": (0, ImportStubs.get_process_heap),
    "kernel32.dll!HeapAlloc": (3, ImportStubs.heap_alloc),
    "kernel32.dll!HeapFree": (3, ImportStubs.heap_free),
    "kernel32.dll!HeapReAlloc": (4, ImportStubs.heap_realloc),
    "kernel32.dll!HeapSize": (3, ImportStubs.heap_size),
    "kernel32.dll!OutputDebugStringA": (1, ImportStubs.output_debug_string),
    "kernel32.dll!GetTickCount": (0, ImportStubs.tick),
    "kernel32.dll!QueryPerformanceCounter": (1, ImportStubs.qpc_counter),
    "kernel32.dll!QueryPerformanceFrequency": (1, ImportStubs.qpc_frequency),
    "kernel32.dll!Sleep": (1, ImportStubs.sleep),
    "kernel32.dll!GlobalAlloc": (2, ImportStubs.global_alloc),
    "kernel32.dll!GlobalFree": (1, ImportStubs.global_free),
    "kernel32.dll!GlobalHandle": (1, ImportStubs.global_handle),
    "kernel32.dll!GlobalMemoryStatus": (1, ImportStubs.global_memory_status),
    "kernel32.dll!GetLocalTime": (1, ImportStubs.local_time),
    "kernel32.dll!GetSystemTime": (1, ImportStubs.local_time),
    "kernel32.dll!CreateFileA": (7, ImportStubs.create_file),
    "kernel32.dll!ReadFile": (5, ImportStubs.read_file),
    "kernel32.dll!WriteFile": (5, ImportStubs.write_file),
    "kernel32.dll!SetFilePointer": (4, ImportStubs.set_file_pointer),
    "kernel32.dll!GetFileSize": (2, ImportStubs.get_file_size),
    "kernel32.dll!CloseHandle": (1, ImportStubs.close_handle),
    "kernel32.dll!FindFirstFileA": (2, ImportStubs.find_first_file),
    "kernel32.dll!FindNextFileA": (2, ImportStubs.find_next_file),
    "kernel32.dll!FindClose": (1, ImportStubs.find_close),
    "kernel32.dll!DeleteFileA": (1, ImportStubs.path_op),
    "kernel32.dll!SetCurrentDirectoryA": (1, ImportStubs.path_op),
    "kernel32.dll!CreateDirectoryA": (2, ImportStubs.create_directory),
    "kernel32.dll!GetFileAttributesA": (1, ImportStubs.file_attributes),
    "kernel32.dll!GetCurrentDirectoryA": (2, ImportStubs.current_directory),
    "user32.dll!MapVirtualKeyA": (2, ImportStubs.map_virtual_key),
    "user32.dll!MessageBoxA": (4, ImportStubs.message_box),
    "user32.dll!MessageBoxExA": (5, ImportStubs.message_box_ex),
    "user32.dll!PostMessageA": (4, ImportStubs.post_message),
    "user32.dll!GetKeyNameTextA": (3, ImportStubs.key_name),
    "user32.dll!LoadCursorA": (2, ImportStubs.load_cursor),
    "user32.dll!SetCursor": (1, ImportStubs.set_cursor),
    "user32.dll!ShowCursor": (1, ImportStubs.show_cursor),
    "kernel32.dll!GetLogicalDrives": (0, ImportStubs.logical_drives),
    "kernel32.dll!GetDriveTypeA": (1, ImportStubs.drive_type),
    "gdi32.dll!SelectObject": (2, ImportStubs.select_object),
    "gdi32.dll!SetTextColor": (2, ImportStubs.set_color),
    "gdi32.dll!SetBkColor": (2, ImportStubs.set_color),
    "gdi32.dll!GetTextExtentPoint32A": (4, ImportStubs.text_extent),
    "winmm.dll!timeGetTime": (0, ImportStubs.tick),
    "kernel32.dll!InitializeCriticalSection": (1, ImportStubs.cs_init),
    "kernel32.dll!EnterCriticalSection": (1, ImportStubs.cs_enter),
    "kernel32.dll!LeaveCriticalSection": (1, ImportStubs.cs_leave),
    "kernel32.dll!DeleteCriticalSection": (1, ImportStubs.cs_delete),
    "kernel32.dll!InterlockedIncrement": (1, ImportStubs.interlocked_inc),
    "kernel32.dll!InterlockedDecrement": (1, ImportStubs.interlocked_dec),
    "kernel32.dll!InterlockedExchange": (2, ImportStubs.interlocked_xchg),
    "kernel32.dll!GetCurrentThreadId": (0, ImportStubs.thread_id),
    "kernel32.dll!GetCurrentProcessId": (0, ImportStubs.process_id),
    "kernel32.dll!TlsAlloc": (0, ImportStubs.tls_alloc),
    "kernel32.dll!TlsGetValue": (1, ImportStubs.tls_get),
    "kernel32.dll!TlsSetValue": (2, ImportStubs.tls_set),
    "kernel32.dll!TlsFree": (1, ImportStubs.tls_free),
    "kernel32.dll!GetLastError": (0, ImportStubs.get_last_error),
    "kernel32.dll!SetLastError": (1, ImportStubs.set_last_error),
    "kernel32.dll!VirtualAlloc": (4, ImportStubs.virtual_alloc),
    "kernel32.dll!VirtualFree": (3, ImportStubs.virtual_free),
    "kernel32.dll!MultiByteToWideChar": (6, ImportStubs.mb_to_wc),
    "kernel32.dll!WideCharToMultiByte": (8, ImportStubs.wc_to_mb),
    "kernel32.dll!GetStdHandle": (1, ImportStubs.std_handle),
    "kernel32.dll!GetModuleFileNameA": (3, ImportStubs.module_file_name),
    "d3d8.dll!Direct3DCreate8": (1, ImportStubs.direct3d_create),
    "dinput8.dll!DirectInput8Create": (5, ImportStubs.direct_input_create),
    "dsound.dll!#11": (3, ImportStubs.direct_sound_create),       # DirectSoundCreate8
}


def first_difference(a: list[tuple], b: list[tuple]) -> str | None:
    """Describe the first difference between two stub call logs, or None when equal."""
    if a == b:
        return None
    for k, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return f"import call #{k}: original {_show(x)} vs reconstruction {_show(y)}"
    k = min(len(a), len(b))
    longer, who = (a, "original") if len(a) > len(b) else (b, "reconstruction")
    return f"import calls: only the {who} makes call #{k} {_show(longer[k])} ({len(a)} vs {len(b)} calls)"


def _show(entry: tuple) -> str:
    name, *args = entry
    return name + "(" + ", ".join(repr(v) if isinstance(v, str) else f"{v:#x}" for v in args) + ")"
