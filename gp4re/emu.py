"""Differential emulation oracle (verification tier T1).

Maps GP4.exe *and* the freshly built reconstruction DLL into one Unicorn x86-32
address space, then for N generated input cases:

  1. runs the ORIGINAL function at its GP4 address,
  2. resets memory, runs the RECONSTRUCTION with identical inputs,
  3. compares return values / declared register + x87 outputs / flags and every
     byte of memory either run touched (GP4 globals, argument buffers).

Reconstructions call not-yet-reconstructed GP4 functions through their original
addresses, so partial reconstructions are testable from day one. Calls into the
Windows API land on trap stubs and make a case *external* (inconclusive): such
functions get ``emu=skip`` and are verified in-process by the hook DLL instead.

Only in-scope GP4 code ever executes here.
"""
from __future__ import annotations

import json
import math
import random
import struct
import time
from dataclasses import dataclass, field
from pathlib import Path

from unicorn import (UC_ARCH_X86, UC_HOOK_CODE, UC_HOOK_MEM_UNMAPPED, UC_HOOK_MEM_WRITE,
                     UC_MODE_32, UC_PROT_ALL, Uc, UcError)
from unicorn import x86_const as X

from .binary import Binary
from .spec import FLAG_BITS, GPRS, Arg, ArgType, Spec

PAGE = 0x1000
STACK_LO, STACK_SIZE = 0x0E000000, 0x00200000
SCRATCH, SCRATCH_SIZE = 0x20000000, 0x01000000
REGS_ADDR = 0x2FFF0000          # gp4::Regs block for regs-convention reconstructions
TRAP, TRAP_SIZE = 0x7FF00000, 0x00010000
SENTINEL = TRAP                 # final stop address
STUB = TRAP + 0x100
CW_SLOT = TRAP + 0x0F00
ST_IN = TRAP + 0x0800
ST_OUT = TRAP + 0x0900
ST_EXT_OUT = TRAP + 0x0B00
FLAGS_SLOT = TRAP + 0x0A00
IMPORT_TRAPS = TRAP + 0x1000
GDT_ADDR, TIB_ADDR = 0x7FFC0000, 0x7FFD0000
REGS_SIZE = 0x68
CANARY = {r: 0xC0DE0000 | (i << 8) for i, r in enumerate(GPRS)}
GPR_ID = {"eax": X.UC_X86_REG_EAX, "ebx": X.UC_X86_REG_EBX, "ecx": X.UC_X86_REG_ECX,
          "edx": X.UC_X86_REG_EDX, "esi": X.UC_X86_REG_ESI, "edi": X.UC_X86_REG_EDI,
          "ebp": X.UC_X86_REG_EBP}
LAYOUT_DIR = Path(__file__).resolve().parent.parent / "specs" / "layouts"


def _align(n: int, a: int = PAGE) -> int:
    return (n + a - 1) & ~(a - 1)


@dataclass
class Impl:
    addr: int
    impl: int
    name: str
    spec: str


def read_registry(dll: Binary) -> list[Impl]:
    """Parse the .gp4reg section written by GP4_IMPL(...) records."""
    sec = dll.section(".gp4reg")
    if not sec:
        return []
    raw = dll.read(sec.va, sec.vsize)
    out = []
    for o in range(0, len(raw) - 15, 4):
        addr, impl, name_p, spec_p = struct.unpack_from("<IIII", raw, o)
        if addr and dll.in_image(impl) and dll.in_image(name_p) and dll.in_image(spec_p) and 0x400000 <= addr < 0x10000000:
            out.append(Impl(addr, impl, _cstr(dll, name_p), _cstr(dll, spec_p)))
    # records are 16 bytes but the linker may pad; de-duplicate by addr
    seen, uniq = set(), []
    for r in out:
        if r.addr not in seen:
            seen.add(r.addr)
            uniq.append(r)
    return uniq


def _cstr(b: Binary, va: int) -> str:
    raw = b.read(va, 512)
    return raw[: raw.find(b"\0")].decode("latin1")


@dataclass
class RunResult:
    status: str                      # ok | fault | external | timeout | error
    detail: str = ""
    regs: dict = field(default_factory=dict)
    st: list = field(default_factory=list)
    st_bits: list[bytes] = field(default_factory=list)
    st_extended: list[bytes] = field(default_factory=list)
    flags: int = 0
    esp: int = 0
    stack_delta: int = 0
    caller_stack: bytes = b""
    caller_stack_before: bytes = b""
    st_depth: int = 0
    fpucw: int = 0
    dirty: dict = field(default_factory=dict)   # page -> bytes
    imports: list = field(default_factory=list)
    insns: int = 0
    blocks: set[int] = field(default_factory=set)
    branches: set[tuple[int, bool]] = field(default_factory=set)


@dataclass
class Coverage:
    """Static target-body obligations; callees are never coverage credit."""
    insns: set[int]
    blocks: set[int]
    branches: dict[int, tuple[int, int]]  # conditional address -> (taken, fallthrough)
    constants: list[int] = field(default_factory=list)


def target_coverage(gp4: Binary, addr: int, function_starts: set[int] | None = None) -> Coverage:
    from .analyze import Analyzer, COND_EXTRA, TERMINATORS, _int
    analyzer = Analyzer(gp4, log=lambda *x: None)
    analyzer.starts = function_starts or set()
    body = analyzer.explore(addr)
    if body.invalid or not body.insns:
        raise ValueError("target body cannot be decoded completely for coverage")
    blocks, branches, constants = {addr}, {}, set()
    for va, ins in body.insns.items():
        nxt, dest = va + ins.size, _int(ins.ops)
        conditional = (ins.mn.startswith("j") and ins.mn != "jmp") or ins.mn in COND_EXTRA
        if conditional:
            if dest is None:
                raise ValueError(f"conditional target unresolved at {va:#x}")
            branches[va] = (dest, nxt)
        if conditional or ins.mn == "jmp":
            blocks.update(v for v in (dest, nxt) if v in body.insns)
        if ins.mn in TERMINATORS and nxt in body.insns:
            blocks.add(nxt)
        if ins.mn == "cmp":
            imm = _int(ins.ops.rsplit(",", 1)[-1].strip())
            if imm is not None:
                constants.update((imm - 1, imm, imm + 1))
    for table, count in body.jtables:
        blocks.update(gp4.u32(table + 4 * k) for k in range(count)
                      if gp4.u32(table + 4 * k) in body.insns)
    return Coverage(set(body.insns), blocks, branches, sorted(constants))


class Machine:
    def __init__(self, gp4: Binary, dll: Binary | None, fpucw: int = 0x027F):
        self.gp4, self.dll = gp4, dll
        self.uc = uc = Uc(UC_ARCH_X86, UC_MODE_32)
        self.regions: list[tuple[int, int]] = []
        self._map(gp4.base, gp4.image)
        if dll:
            self._map(dll.base, dll.image)
        self._map(STACK_LO, bytes(STACK_SIZE))
        self._map(SCRATCH, bytes(SCRATCH_SIZE))
        self._map(REGS_ADDR & ~0xFFFF, bytes(0x10000))
        self._map(TRAP, b"\xF4" * TRAP_SIZE)
        self.import_names: dict[int, str] = {}
        for i, (slot, name) in enumerate(sorted(gp4.imports.items())):
            t = IMPORT_TRAPS + 4 * i
            self.import_names[t] = name
            uc.mem_write(slot, struct.pack("<I", t))
        self._setup_fs()
        self.pristine = {a: bytes(uc.mem_read(a, n)) for a, n in self.regions}
        self.dirty: set[int] = set()
        self.fault: str | None = None
        self.external: list[str] = []
        self.fpucw = fpucw
        self.coverage: Coverage | None = None
        self.trace_original = False
        self.trace_blocks: set[int] = set()
        self.trace_branches: set[tuple[int, bool]] = set()
        self.pending_branch: tuple[int, int, int] | None = None
        self.insns = 0
        self.return_pc: int | None = None
        self.return_fp: tuple[int, int] | None = None
        self.excluded_code = [(s.va, s.end) for s in gp4.sections if s.executable and not s.in_scope]
        uc.hook_add(UC_HOOK_MEM_WRITE, self._on_write)
        uc.hook_add(UC_HOOK_MEM_UNMAPPED, self._on_unmapped)
        uc.hook_add(UC_HOOK_CODE, self._on_import, begin=IMPORT_TRAPS, end=TRAP + TRAP_SIZE - 1)
        uc.hook_add(UC_HOOK_CODE, self._on_code)

    def run_init(self, addrs: list[int]) -> list[str]:
        """Run GP4's own initialisers once and make the result the pristine state."""
        problems = []
        for a in addrs:
            r = self.call(a, regs={}, stack=b"", st_in=[], n_st_out=0, capture_flags=False, maxinsn=2_000_000)
            if r.status != "ok":
                problems.append(f"init {a:#x}: {r.status} {r.detail}")
        self.pristine = {a: bytes(self.uc.mem_read(a, n)) for a, n in self.regions}
        self.dirty.clear()
        self.fault, self.external = None, []
        return problems

    # ------------------------------------------------------------ setup
    def _map(self, base: int, data: bytes | bytearray):
        size = _align(len(data))
        self.uc.mem_map(base, size, UC_PROT_ALL)
        self.uc.mem_write(base, bytes(data))
        self.regions.append((base, size))

    def _setup_fs(self):
        uc = self.uc
        uc.mem_map(GDT_ADDR, PAGE, UC_PROT_ALL)
        uc.mem_map(TIB_ADDR, PAGE, UC_PROT_ALL)
        tib = struct.pack("<IIIIIII", 0xFFFFFFFF, STACK_LO + STACK_SIZE, STACK_LO, 0, 0, 0, TIB_ADDR)
        uc.mem_write(TIB_ADDR, tib)
        self.regions += [(GDT_ADDR, PAGE), (TIB_ADDR, PAGE)]

        def desc(base, limit, access, flags):
            return (limit & 0xFFFF) | ((base & 0xFFFFFF) << 16) | (access << 40) | \
                (((limit >> 16) & 0xF) << 48) | (flags << 52) | (((base >> 24) & 0xFF) << 56)
        gdt = [0, desc(0, 0xFFFFF, 0x9A, 0xC), desc(0, 0xFFFFF, 0x92, 0xC), desc(TIB_ADDR, 0xFFF, 0xF2, 0x4)]
        uc.mem_write(GDT_ADDR, b"".join(struct.pack("<Q", d) for d in gdt))
        uc.reg_write(X.UC_X86_REG_GDTR, (0, GDT_ADDR, len(gdt) * 8 - 1, 0))
        # flat 32-bit code/data/stack segments; FS -> a fake TIB (SEH frames read fs:[0])
        for reg, sel in ((X.UC_X86_REG_CS, 1 << 3), (X.UC_X86_REG_DS, 2 << 3), (X.UC_X86_REG_ES, 2 << 3),
                         (X.UC_X86_REG_SS, 2 << 3)):
            uc.reg_write(reg, sel)
        try:
            uc.reg_write(X.UC_X86_REG_FS, (3 << 3) | 3)
            self.fs_ok = True
        except UcError:
            self.fs_ok = False

    # ------------------------------------------------------------ hooks
    def _on_write(self, uc, access, address, size, value, user):
        self.dirty.add(address & ~(PAGE - 1))
        if (address + size - 1) & ~(PAGE - 1) != address & ~(PAGE - 1):
            self.dirty.add((address + size - 1) & ~(PAGE - 1))

    def _on_unmapped(self, uc, access, address, size, value, user):
        self.fault = f"unmapped access at {address:#010x} (eip={uc.reg_read(X.UC_X86_REG_EIP):#010x})"
        return False

    def _on_import(self, uc, address, size, user):
        if address in self.import_names:
            self.external.append(self.import_names[address])
            uc.emu_stop()

    def _on_code(self, uc, address, size, user):
        self.insns += 1
        if address == self.return_pc:
            tag = uc.reg_read(X.UC_X86_REG_FPTAG)
            self.return_fp = (sum(((tag >> (2 * k)) & 3) != 3 for k in range(8)),
                              uc.reg_read(X.UC_X86_REG_FPCW))
        if any(a <= address < b for a, b in self.excluded_code):
            self.fault = f"out-of-scope executable section at {address:#010x}"
            uc.emu_stop()
            return
        cov = self.coverage if self.trace_original else None
        if cov is None:
            return
        if self.pending_branch is not None:
            va, taken, fallthrough = self.pending_branch
            if address == taken:
                self.trace_branches.add((va, True))
                if taken == fallthrough:    # both directions land on the next insn: indistinguishable
                    self.trace_branches.add((va, False))
            elif address == fallthrough:
                self.trace_branches.add((va, False))
            self.pending_branch = None
        if address in cov.blocks:
            self.trace_blocks.add(address)
        if address in cov.branches:
            taken, fallthrough = cov.branches[address]
            self.pending_branch = (address, taken, fallthrough)

    # ------------------------------------------------------------ run
    def reset(self):
        for page in self.dirty:
            for a, n in self.regions:
                if a <= page < a + n:
                    o = page - a
                    self.uc.mem_write(page, self.pristine[a][o: o + PAGE])
                    break
        self.dirty.clear()
        self.fault = None
        self.external = []

    def call(self, target: int, *, regs: dict[str, int], stack: bytes, st_in: list[float],
             n_st_out: int, capture_flags: bool, maxinsn: int, input_flags: int = 0x202,
             stack_noise: bytes | None = None) -> RunResult:
        uc = self.uc
        code = bytearray()
        code += b"\xDB\xE3"                                   # fninit
        code += b"\xD9\x2D" + struct.pack("<I", CW_SLOT)      # fldcw [cw]
        for k in reversed(range(len(st_in))):                 # st(n-1) first, st0 last
            code += b"\xDD\x05" + struct.pack("<I", ST_IN + 8 * k)
        call_at = STUB + len(code)
        code += b"\xE8" + struct.pack("<i", target - (call_at + 5))
        self.return_pc, self.return_fp = call_at + 5, None
        if capture_flags:
            code += b"\x9C" + b"\x8F\x05" + struct.pack("<I", FLAGS_SLOT)   # pushfd; pop [slot]
        for k in range(n_st_out):
            code += b"\xDB\x3D" + struct.pack("<I", ST_EXT_OUT + 16 * k)  # fstp tbyte [raw]
            code += b"\xDB\x2D" + struct.pack("<I", ST_EXT_OUT + 16 * k)  # fld tbyte [raw]
            code += b"\xDD\x1D" + struct.pack("<I", ST_OUT + 8 * k)      # fstp qword [slot]
        jmp_at = STUB + len(code)
        code += b"\xE9" + struct.pack("<i", SENTINEL - (jmp_at + 5))
        uc.mem_write(STUB, bytes(code))
        uc.ctl_remove_cache(STUB, STUB + 0x300)   # up to eight raw x87 outputs
        uc.mem_write(CW_SLOT, struct.pack("<H", self.fpucw))
        for k, v in enumerate(st_in):
            uc.mem_write(ST_IN + 8 * k, struct.pack("<d", v))
        esp = STACK_LO + STACK_SIZE - 0x1000 - _align(len(stack), 16)
        if stack_noise is not None:
            # Vary undeclared caller-stack slots and uninitialised locals while
            # retaining the supplied arguments and the stub's return address.
            lo = esp - PAGE
            uc.mem_write(lo, stack_noise[: STACK_LO + STACK_SIZE - lo])
            self.dirty.update(range(lo & ~(PAGE - 1), STACK_LO + STACK_SIZE, PAGE))
        uc.mem_write(esp, stack)
        caller_stack_before = bytes(uc.mem_read(esp, STACK_LO + STACK_SIZE - esp))
        for r in GPRS:
            uc.reg_write(GPR_ID[r], regs.get(r, CANARY[r]))
        uc.reg_write(X.UC_X86_REG_ESP, esp)
        uc.reg_write(X.UC_X86_REG_EFLAGS, input_flags)
        self.trace_blocks, self.trace_branches = set(), set()
        self.pending_branch, self.insns = None, 0
        self.dirty.update(range(esp & ~(PAGE - 1), STACK_LO + STACK_SIZE, PAGE))
        res = RunResult(status="ok")
        try:
            uc.emu_start(STUB, SENTINEL, count=maxinsn)
        except UcError as e:
            res.status, res.detail = "fault", self.fault or str(e)
        eip = uc.reg_read(X.UC_X86_REG_EIP)
        if self.fault:
            res.status, res.detail = "fault", self.fault
        elif res.status == "ok" and self.external:
            res.status, res.detail = "external", self.external[0]
        elif res.status == "ok" and eip != SENTINEL:
            res.status, res.detail = "timeout", f"stopped at {eip:#010x} after instruction budget"
        if self.fault and res.status == "ok":
            res.status, res.detail = "fault", self.fault
        res.regs = {r: uc.reg_read(GPR_ID[r]) for r in GPRS}
        res.st_bits = [bytes(uc.mem_read(ST_OUT + 8 * k, 8)) for k in range(n_st_out)]
        res.st_extended = [bytes(uc.mem_read(ST_EXT_OUT + 16 * k, 10)) for k in range(n_st_out)]
        res.st = [struct.unpack("<d", v)[0] for v in res.st_bits]
        res.esp = uc.reg_read(X.UC_X86_REG_ESP)
        res.stack_delta = res.esp - esp
        if self.return_fp is not None:
            res.st_depth, res.fpucw = self.return_fp
        if esp <= res.esp <= STACK_LO + STACK_SIZE:
            res.caller_stack = bytes(uc.mem_read(res.esp, STACK_LO + STACK_SIZE - res.esp))
            res.caller_stack_before = caller_stack_before[res.stack_delta:]
        if capture_flags:
            res.flags = struct.unpack("<I", bytes(uc.mem_read(FLAGS_SLOT, 4)))[0]
        res.imports = list(self.external)
        res.insns = self.insns
        res.blocks, res.branches = self.trace_blocks.copy(), self.trace_branches.copy()
        return res

    def snapshot_dirty(self, exclude: list[tuple[int, int]]) -> dict[int, bytes]:
        out = {}
        for p in self.dirty:
            if any(a <= p < b for a, b in exclude):
                continue
            out[p] = bytes(self.uc.mem_read(p, PAGE))
        return out


# ---------------------------------------------------------------- input generation
class CaseGen:
    def __init__(self, spec: Spec, seed: int):
        self.spec = spec
        self.rng = random.Random(seed)
        self.next_buf = SCRATCH
        self.buffers: list[tuple[int, bytes]] = []
        self.constants: list[int] = []

    def alloc(self, data: bytes) -> int:
        a = self.next_buf
        self.next_buf = _align(a + len(data) + PAGE)  # guard gap between buffers
        self.buffers.append((a, data))
        return a

    def scalar(self, t: ArgType):
        r = self.rng
        if t.kind in ("f32", "f64"):
            if t.lo is not None:
                return r.uniform(t.lo, t.hi)
            return r.choice([0.0, 1.0, -1.0, 0.5, r.uniform(-1, 1), r.uniform(-1, 1), r.uniform(-1000, 1000),
                             r.uniform(-1e6, 1e6), r.uniform(-1, 1) * 1e-6])
        bits = {"i8": 8, "u8": 8, "i16": 16, "u16": 16, "i32": 32, "u32": 32}[t.kind]
        low = int(t.lo) if t.lo is not None else (0 if t.kind.startswith("u") else -(1 << (bits - 1)))
        high = int(t.hi) if t.hi is not None else ((1 << bits) - 1 if t.kind.startswith("u") else (1 << (bits - 1)) - 1)
        directed = [v for v in self.constants if low <= v <= high]
        if directed and r.randrange(2):
            v = r.choice(directed)
        elif t.lo is not None:
            edges = sorted({low, high} | {v for v in (-1, 0, 1) if low <= v <= high})
            v = r.choice(edges) if r.randrange(3) == 0 else r.randint(low, high)
        else:
            v = r.choice([0, 1, -1, 2, r.randint(-100, 100), r.randint(0, 255), r.randint(-2 ** 15, 2 ** 15),
                          r.getrandbits(bits)])
        return v & ((1 << bits) - 1) if t.kind.startswith("u") else v

    def fill(self, size: int, fill: str) -> bytes:
        r = self.rng
        if fill == "zero":
            return bytes(size)
        if fill == "bytes":
            return r.randbytes(size)
        if fill == "f32":
            return b"".join(struct.pack("<f", r.uniform(-100, 100)) for _ in range(size // 4)).ljust(size, b"\0")
        if fill == "f64":
            return b"".join(struct.pack("<d", r.uniform(-100, 100)) for _ in range(size // 8)).ljust(size, b"\0")
        if fill == "i32":
            return b"".join(struct.pack("<i", r.randint(-1000, 1000)) for _ in range(size // 4)).ljust(size, b"\0")
        if fill.startswith("layout="):
            return self.layout(fill.split("=", 1)[1], size)
        raise ValueError(fill)

    def layout(self, name: str, size: int) -> bytes:
        spec = json.loads((LAYOUT_DIR / f"{name}.json").read_text())
        buf = bytearray(max(size, spec.get("size", 0)))
        for f in spec["fields"]:
            off, ty = f["off"], f["type"]
            t = ArgType(ty if ty != "ptr" else "ptr", f.get("lo"), f.get("hi"), f.get("size", 0), f.get("fill", "bytes"))
            if ty == "ptr":
                struct.pack_into("<I", buf, off, self.alloc(self.fill(t.size, t.fill)))
            elif ty == "f32":
                struct.pack_into("<f", buf, off, self.scalar(t))
            elif ty == "f64":
                struct.pack_into("<d", buf, off, self.scalar(t))
            else:
                n = {"i8": 1, "u8": 1, "i16": 2, "u16": 2, "i32": 4, "u32": 4}[ty]
                buf[off: off + n] = (self.scalar(t) & ((1 << (8 * n)) - 1)).to_bytes(n, "little")
        return bytes(buf)

    def value(self, a: Arg):
        """Return ('int', u32) | ('f32', float) | ('f64', float)."""
        t = a.t
        if t.kind == "ptr":
            return ("int", self.alloc(self.fill(t.size, t.fill)))
        v = self.scalar(t)
        if t.kind in ("f32", "f64"):
            return (t.kind, v)
        return ("int", v & 0xFFFFFFFF)


def _push(vals) -> bytes:
    out = b""
    for kind, v in vals:
        if kind == "int":
            out += struct.pack("<I", v)
        elif kind == "f32":
            out += struct.pack("<f", v)
        else:
            out += struct.pack("<d", v)
    return out


# ---------------------------------------------------------------- comparison
def _close(a: float, b: float, tol: float) -> bool:
    if a == b:
        return True
    return tol > 0 and abs(a - b) <= tol * max(1.0, abs(a), abs(b))


def _float_bits(raw: bytes, width: int = 8) -> bytes:
    if width == 8:
        return raw
    value = struct.unpack("<d", raw)[0]
    try:
        return struct.pack("<f", value)
    except OverflowError:
        return struct.pack("<f", float("-inf") if value < 0 else float("inf"))


def _float_equal(a: bytes, b: bytes, tol: float, width: int = 8) -> bool:
    a, b = _float_bits(a, width), _float_bits(b, width)
    if a == b:
        return True
    if tol == 0:
        return False
    fmt = "<f" if width == 4 else "<d"
    left, right = struct.unpack(fmt, a)[0], struct.unpack(fmt, b)[0]
    if (left == 0 and right == 0) or not math.isfinite(left) or not math.isfinite(right):
        return False
    return _close(left, right, tol)


def _double_extended(raw: bytes) -> bytes:
    """Exact x87 extended representation of a binary64 encoding."""
    bits = struct.unpack("<Q", raw)[0]
    sign, exponent, fraction = bits >> 63, (bits >> 52) & 0x7FF, bits & ((1 << 52) - 1)
    if exponent == 0x7FF:
        extended_exponent, significand = 0x7FFF, (1 << 63) | (fraction << 11)
    elif exponent:
        extended_exponent, significand = exponent - 1023 + 16383, (1 << 63) | (fraction << 11)
    elif fraction:
        shift = 64 - fraction.bit_length()
        extended_exponent, significand = 15372 - shift, fraction << shift
    else:
        extended_exponent, significand = 0, 0
    return struct.pack("<QH", significand, (sign << 15) | extended_exponent)


def _f32(x: float) -> float:
    try:
        return struct.unpack("<f", struct.pack("<f", x))[0]
    except OverflowError:
        return x


def _fmt(b8: bytes) -> str:
    """Show an 8-byte slot as hex plus its plausible float/int readings."""
    d = struct.unpack("<d", b8)[0]
    f0, f1 = struct.unpack("<ff", b8)
    i0, i1 = struct.unpack("<ii", b8)
    return f"{b8.hex()} (f64 {d:.9g} | f32 {f0:.7g},{f1:.7g} | i32 {i0},{i1})"


def compare_pages(a: dict[int, bytes], b: dict[int, bytes], pristine_of, tol: float) -> list[str]:
    diffs = []
    for p in sorted(set(a) | set(b)):
        pa = a.get(p) or pristine_of(p)
        pb = b.get(p) or pristine_of(p)
        if pa == pb:
            continue
        for o in range(0, PAGE, 8):
            qa, qb = pa[o: o + 8], pb[o: o + 8]
            if qa == qb:
                continue
            # Memory is untyped here. Reinterpreting arbitrary integers/pointers
            # as tiny floats used to hide changes whenever tol was enabled.
            # Tolerance is therefore restricted to explicit floating outputs.
            diffs.append(f"mem {p + o:#010x}: original {_fmt(qa)} vs reconstruction {_fmt(qb)}")
            if len(diffs) >= 8:
                return diffs
    return diffs


# ---------------------------------------------------------------- driver
def differential(gp4: Binary, dll: Binary, rec: Impl, spec: Spec, *, fpucw: int, globals_info: list[int] | None = None,
                 cases: int | None = None, function_starts: set[int] | None = None) -> dict:
    m = Machine(gp4, dll, fpucw=spec.fpucw if spec.fpucw is not None else fpucw)
    from .config import oracle
    init_problems = m.run_init(list(oracle("init_calls")))
    if init_problems:
        return {"gate": "emu", "status": "fail", "reason": "initialization failed", "problems": init_problems}
    if spec.emu_skip:
        return {"gate": "emu", "status": "skipped", "reason": spec.emu_skip}
    try:
        cov = target_coverage(gp4, rec.addr, function_starts)
    except (ValueError, SystemExit) as e:
        return {"gate": "emu", "status": "inconclusive", "reason": f"coverage unavailable: {e}"}
    m.coverage = cov
    n = spec.cases if cases is None else cases
    if n < 1:
        return {"gate": "emu", "status": "fail", "reason": "case count must be positive"}
    limit = max(n, int(oracle("max_cases")))
    deadline = time.monotonic() + float(oracle("time_budget_s"))
    exclude = [(STACK_LO, STACK_LO + STACK_SIZE), (REGS_ADDR & ~0xFFFF, (REGS_ADDR & ~0xFFFF) + 0x10000),
               (TRAP, TRAP + TRAP_SIZE)]
    pristine = {}
    for a, sz in m.regions:
        pristine[a] = (sz, m.pristine[a])

    baseline_pages: dict[int, bytes] = {}

    def pristine_of(p):
        if p in baseline_pages:
            return baseline_pages[p]
        for a, (sz, data) in pristine.items():
            if a <= p < a + sz:
                return data[p - a: p - a + PAGE]
        return bytes(PAGE)

    ret_float = spec.ret in ("f32", "f64")
    n_st_out = 1 if ret_float else len(spec.st_outputs)
    capture_flags = spec.conv == "regs"
    passed = invalid = 0
    invalid_reasons: dict[str, int] = {}
    warnings: set[str] = set()
    seen_blocks: set[int] = set()
    seen_branches: set[tuple[int, bool]] = set()
    ni_checked = 0
    ni_target = max(0, int(oracle("ni_cases")))
    attempted = 0

    def coverage_ok():
        return (len(seen_blocks) / len(cov.blocks) >= float(oracle("min_block_coverage")) and
                (not cov.branches or len(seen_branches) / (2 * len(cov.branches)) >= float(oracle("min_branch_coverage"))))

    for i in range(limit):
        if i >= n and (coverage_ok() or time.monotonic() >= deadline):
            break
        attempted += 1
        seed = (spec.seed * 1_000_003 + rec.addr * 7919 + i) & 0xFFFFFFFF
        g = CaseGen(spec, seed)
        if i >= n:
            g.constants = cov.constants
        flags = 0x202
        if spec.conv == "regs":
            regs, st_in, desc = {}, [], {}
            for a in spec.args:
                kind, v = g.value(a)
                if a.reg.startswith("st"):
                    continue   # load the sorted contiguous stack below
                elif a.reg in FLAG_BITS:
                    flags = (flags & ~(1 << FLAG_BITS[a.reg])) | ((v & 1) << FLAG_BITS[a.reg])
                else:
                    regs[a.reg] = v & 0xFFFFFFFF if kind == "int" else struct.unpack("<I", struct.pack("<f", v))[0]
                desc[a.reg] = v if kind != "int" else hex(v)
            for a in spec.st_inputs:
                kind, v = g.value(a)
                st_in.append(float(v))
                desc[a.reg] = v
            stack_o = b""
        else:
            vals = [g.value(a) for a in spec.args]
            desc = {f"arg{k}": (hex(v) if kind == "int" else v) for k, (kind, v) in enumerate(vals)}
            regs = {}
            stack_vals = list(vals)
            if spec.conv == "thiscall":
                regs["ecx"] = stack_vals.pop(0)[1]
            elif spec.conv == "fastcall":
                for r in ("ecx", "edx"):
                    if stack_vals and stack_vals[0][0] == "int":
                        regs[r] = stack_vals.pop(0)[1]
            stack_o = _push(stack_vals)
            st_in = []
        if spec.globals.startswith("fuzz") and globals_info:
            kind = spec.globals.split(":", 1)[1] if ":" in spec.globals else "f32"
            for gaddr in globals_info:
                v = g.scalar(ArgType(kind))
                data = struct.pack("<I", v & 0xFFFFFFFF) if kind == "i32" else struct.pack({"f32": "<f", "f64": "<d"}[kind], v)
                g.buffers.append((gaddr, data))
        elif spec.globals.startswith("layout="):
            from .input_layouts import global_buffers
            try:
                g.buffers.extend(global_buffers(g, spec.globals.split("=", 1)[1], gp4))
            except (ValueError, KeyError, OSError, struct.error) as e:
                return {"gate": "emu", "status": "inconclusive", "reason": f"invalid global input layout: {e}"}
        bufs = list(g.buffers)

        def load_inputs():
            for a, data in bufs:
                m.uc.mem_write(a, data)
                m.dirty.update(range(a & ~(PAGE - 1), _align(a + len(data)), PAGE))

        # ---- original
        m.reset()
        load_inputs()
        baseline_pages = m.snapshot_dirty(exclude)
        m.trace_original = True
        orig = m.call(rec.addr, regs=regs, stack=stack_o, st_in=st_in, n_st_out=n_st_out,
                      capture_flags=capture_flags, maxinsn=spec.maxinsn, input_flags=flags)
        if orig.status != "ok":
            invalid += 1
            key = orig.status + (": " + orig.detail if orig.status == "external" else "")
            invalid_reasons[key] = invalid_reasons.get(key, 0) + 1
            continue
        orig.dirty = m.snapshot_dirty(exclude)
        if orig.st_depth != n_st_out:
            return _fail(rec, i, desc, f"x87 output contract declares {n_st_out} values but original returns {orig.st_depth}", passed, invalid)
        if spec.conv == "regs" and any(raw != _double_extended(value)
                                       for raw, value in zip(orig.st_extended, orig.st_bits)):
            return {"gate": "emu", "status": "inconclusive", "reason": "original x87 output cannot be represented exactly by gp4::Regs double slots",
                    "case": i, "inputs": desc, "passed": passed, "invalid": invalid,
                    "st_extended": [v.hex() for v in orig.st_extended], "st_binary64": [v.hex() for v in orig.st_bits]}
        if spec.conv == "regs" and orig.fpucw != m.fpucw:
            return {"gate": "emu", "status": "inconclusive", "reason": "original changes x87 control word, unsupported by gp4::Regs",
                    "case": i, "passed": passed, "invalid": invalid,
                    "fpucw_in": hex(m.fpucw), "fpucw_out": hex(orig.fpucw)}
        seen_blocks.update(orig.blocks)
        seen_branches.update(orig.branches)

        # Missing inputs are tested on the ORIGINAL, independently of whether
        # the reconstruction happens to repeat the same fixed-canary result.
        if ni_checked < ni_target:
            entry = {r: regs.get(r, CANARY[r]) for r in GPRS}
            declared_flags = {a.reg for a in spec.args if a.reg in FLAG_BITS}
            noise_rng = random.Random(seed ^ 0x4E494348)
            variations = [(r, {**regs, r: noise_rng.getrandbits(32)}, flags, None)
                          for r in GPRS if r not in regs]
            variations += [(o, regs, flags ^ (1 << bit), None)
                           for o, bit in FLAG_BITS.items() if o not in declared_flags]
            variations.append(("undeclared stack", regs, flags, noise_rng.randbytes(4 * PAGE)))
            for label, vregs, vflags, noise in variations:
                m.reset()
                load_inputs()
                variant = m.call(rec.addr, regs=vregs, stack=stack_o, st_in=st_in, n_st_out=n_st_out,
                                 capture_flags=capture_flags, maxinsn=spec.maxinsn,
                                 input_flags=vflags, stack_noise=noise)
                if variant.status != "ok":
                    return _fail(rec, i, desc, f"hidden input {label}: varying it makes original {variant.status}: {variant.detail}", passed, invalid)
                variant.dirty = m.snapshot_dirty(exclude)
                ventry = {r: vregs.get(r, CANARY[r]) for r in GPRS}
                dependencies = _ni_problems(orig, variant, spec, entry, ventry, flags, vflags)
                dependencies += compare_pages(orig.dirty, variant.dirty, pristine_of, 0)
                if dependencies:
                    return _fail(rec, i, desc, f"hidden input {label}: " + "; ".join(dependencies[:8]), passed, invalid)
            ni_checked += 1

        # ---- reconstruction
        m.reset()
        load_inputs()
        m.trace_original = False
        if spec.conv == "regs":
            block = bytearray(REGS_SIZE)
            for k, r in enumerate(GPRS):
                struct.pack_into("<I", block, 4 * k, regs.get(r, CANARY[r]))
            struct.pack_into("<I", block, 0x1C, flags)
            for k, v in enumerate(st_in):
                struct.pack_into("<d", block, 0x20 + 8 * k, v)
            struct.pack_into("<ii", block, 0x60, len(st_in), len(spec.st_outputs))
            m.uc.mem_write(REGS_ADDR, bytes(block))
            m.dirty.add(REGS_ADDR & ~(PAGE - 1))
            recon = m.call(rec.impl, regs={}, stack=struct.pack("<I", REGS_ADDR), st_in=[], n_st_out=0,
                           capture_flags=False, maxinsn=spec.maxinsn * 4, input_flags=flags)
            out_block = bytes(m.uc.mem_read(REGS_ADDR, REGS_SIZE))
        else:
            recon = m.call(rec.impl, regs=regs, stack=stack_o, st_in=[], n_st_out=n_st_out,
                           capture_flags=False, maxinsn=spec.maxinsn * 4, input_flags=flags)
        if recon.status != "ok":
            return _fail(rec, i, desc, f"reconstruction {recon.status}: {recon.detail}", passed, invalid)
        recon.dirty = m.snapshot_dirty(exclude)

        # ---- compare
        problems = []
        if spec.conv == "regs":
            for k, r in enumerate(GPRS):
                rv = struct.unpack_from("<I", out_block, 4 * k)[0]
                if rv != orig.regs[r]:
                    problems.append(f"{r}: original {orig.regs[r]:#010x} vs reconstruction {rv:#010x}")
            for k, o in enumerate(spec.st_outputs):
                raw = out_block[0x20 + 8 * k: 0x28 + 8 * k]
                rv = struct.unpack("<d", raw)[0]
                if not _float_equal(orig.st_bits[k], raw, spec.tol):
                    problems.append(f"{o}: original {orig.st[k]!r} vs reconstruction {rv!r}")
            rflags = struct.unpack_from("<I", out_block, 0x1C)[0]
            for o, bit in FLAG_BITS.items():
                if ((orig.flags >> bit) & 1) != ((rflags >> bit) & 1):
                    problems.append(f"{o}: original {(orig.flags >> bit) & 1} vs reconstruction {(rflags >> bit) & 1}")
        elif spec.ret in ("i32", "u32"):
            if orig.regs["eax"] != recon.regs["eax"]:
                problems.append(f"return eax: original {orig.regs['eax']:#010x} vs reconstruction {recon.regs['eax']:#010x}")
        elif spec.ret == "i64":
            if (orig.regs["eax"], orig.regs["edx"]) != (recon.regs["eax"], recon.regs["edx"]):
                problems.append("return edx:eax differs")
        elif ret_float:
            a, b = orig.st[0], recon.st[0]
            equal = (orig.st_extended[0] == recon.st_extended[0] if spec.tol == 0 else
                     _float_equal(orig.st_bits[0], recon.st_bits[0], spec.tol, 4 if spec.ret == "f32" else 8))
            if not equal:
                problems.append(f"return st0: original {a!r} vs reconstruction {b!r}")
        if spec.conv != "regs":
            if orig.st_depth != recon.st_depth:
                problems.append(f"x87 stack depth: original {orig.st_depth} vs reconstruction {recon.st_depth}")
            if orig.fpucw != recon.fpucw:
                problems.append(f"x87 control word: original {orig.fpucw:#x} vs reconstruction {recon.fpucw:#x}")
            for r in ("ebx", "esi", "edi", "ebp"):
                if orig.regs[r] != recon.regs[r]:
                    problems.append(f"callee-saved {r}: original {orig.regs[r]:#010x} vs reconstruction {recon.regs[r]:#010x}")
        if orig.stack_delta != recon.stack_delta:
            problems.append(f"stack balance: original ESP delta {orig.stack_delta} vs reconstruction {recon.stack_delta}")
        if spec.conv == "regs":
            # The C wrapper has one extra stack argument below the original's
            # caller-owned interval; compare the same absolute live addresses.
            live_recon = bytes(m.uc.mem_read(orig.esp, len(orig.caller_stack)))
        else:
            live_recon = recon.caller_stack
        if orig.caller_stack != live_recon:
            problems.append("caller-owned stack memory differs")
        problems += compare_pages(orig.dirty, recon.dirty, pristine_of, spec.tol)
        if problems:
            return _fail(rec, i, desc, "; ".join(problems[:12]), passed, invalid)
        passed += 1

    status = "pass" if passed and invalid == 0 and coverage_ok() and ni_checked >= min(ni_target, passed) else "inconclusive"
    if passed == 0:
        status = "inconclusive"
    coverage = {"blocks_hit": len(seen_blocks), "blocks_total": len(cov.blocks),
                "block_fraction": len(seen_blocks) / len(cov.blocks),
                "branch_directions_hit": len(seen_branches), "branch_directions_total": 2 * len(cov.branches),
                "branch_fraction": len(seen_branches) / (2 * len(cov.branches)) if cov.branches else 1.0,
                "missing_blocks": [hex(a) for a in sorted(cov.blocks - seen_blocks)][:20],
                "missing_branch_directions": [f"{va:#x}:{'taken' if taken else 'fallthrough'}"
                                              for va in sorted(cov.branches) for taken in (False, True)
                                              if (va, taken) not in seen_branches][:20]}
    return {"gate": "emu", "status": status, "cases": attempted, "requested_cases": n,
            "passed": passed, "invalid": invalid, "coverage": coverage, "ni_checked": ni_checked,
            "invalid_reasons": invalid_reasons, "warnings": sorted(warnings), "fs_ok": m.fs_ok,
            **({"reason": "insufficient target-body coverage"} if not coverage_ok() else {})}


def _ni_problems(base: RunResult, variant: RunResult, spec: Spec, entry: dict, ventry: dict,
                 flags: int, vflags: int) -> list[str]:
    problems = []
    if spec.conv == "regs":
        for r in GPRS:
            # A preserved register may naturally reflect its changed entry
            # value; a value computed from another omitted input may not.
            if base.regs[r] == entry[r] and variant.regs[r] == ventry[r]:
                continue
            if base.regs[r] != variant.regs[r]:
                problems.append(f"output {r} changes")
        for o, bit in FLAG_BITS.items():
            a, b = (base.flags >> bit) & 1, (variant.flags >> bit) & 1
            if a == ((flags >> bit) & 1) and b == ((vflags >> bit) & 1):
                continue
            if a != b:
                problems.append(f"output {o} changes")
    else:
        ret_regs = ("eax", "edx") if spec.ret == "i64" else (("eax",) if spec.ret in ("i32", "u32") else ())
        for r in ret_regs:
            if base.regs[r] != variant.regs[r]:
                problems.append(f"return {r} changes")
        for r in ("ebx", "esi", "edi", "ebp"):
            if base.regs[r] == entry[r] and variant.regs[r] == ventry[r]:
                continue
            if base.regs[r] != variant.regs[r]:
                problems.append(f"callee-saved {r} changes")
    for k, (a, b) in enumerate(zip(base.st_extended, variant.st_extended)):
        if a != b:
            problems.append(f"output st{k} changes")
    if base.stack_delta != variant.stack_delta:
        problems.append("stack balance changes")
    if base.st_depth != variant.st_depth or base.fpucw != variant.fpucw:
        problems.append("x87 state changes")
    if len(base.caller_stack) == len(variant.caller_stack):
        for k, (a, b) in enumerate(zip(base.caller_stack, variant.caller_stack)):
            changed = (a != base.caller_stack_before[k] or b != variant.caller_stack_before[k])
            if changed and a != b:
                problems.append(f"caller-stack effect at +{k:#x} changes")
                break
    return problems


def _fail(rec: Impl, case: int, desc: dict, why: str, passed: int, invalid: int) -> dict:
    return {"gate": "emu", "status": "fail", "case": case, "inputs": desc, "mismatch": why,
            "passed_before_failure": passed, "invalid": invalid}
