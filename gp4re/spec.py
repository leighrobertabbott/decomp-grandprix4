"""Calling-convention / test-input spec carried by every reconstruction.

Grammar (whitespace-insensitive)::

    spec    := conv "(" [arg ("," arg)*] ")" "->" ret [";" opt]*
    conv    := cdecl | stdcall | thiscall | fastcall | regs
    arg     := type                       # stack/ecx/edx args for compiled conventions
             | reg ":" type               # regs convention: eax..ebp, st0..st7, arithmetic flags
    type    := i32 | u32 | i16 | u16 | i8 | u8 | f32 | f64 [ "[" lo ".." hi "]" ]
             | ptr "[" size "]" ["+" bias] [":" fill]   # nptr[...]: null in ~1/4 of cases; +bias: the
                                                      # value points bias bytes into the buffer
             | rec "[" base "," stride "," lo ".." hi "]"   # base + k*stride, k in lo..hi
    fill    := f32 | f64 | i32 | bytes | zero | layout=<name> | com=<Interface>   # gp4re/com.py
             | "{" ["*bytes" ","] field ("," field)* "}"     # inline buffer, unlisted bytes zero
    field   := off ["*" count ["/" stride]] ":" type          # e.g. {8:ptr[0x40]:bytes, 0xc:u32[0..15]}
    ret     := void | i32 | u32 | u8 | i8 | u16 | i16 | f32 | f64 | i64 | "(" out ("," out)* ")" | "()"
               # u8/i8 compare al only, u16/i16 ax only (compiled bool/char/short returns that leave
               # the upper eax bits undefined); the emu gate fails them if any direct caller of the
               # original reads eax beyond that width (gp4re/retwidth.py)
    out     := eax|ebx|ecx|edx|esi|edi|ebp|st0..st7|cf|pf|af|zf|sf|of
    opt     := tol=<float> | cases=<n> | fpucw=<hex> | globals=keep|fuzz:<f32|f64|i32>|[fuzz:<k>+]layout=<name>[+<name>...]
             | emu=skip:<reason> | maxinsn=<n> | seed=<n>

Examples::

    cdecl(ptr[0x60]:f64, f32) -> f32
    thiscall(ptr[0x200]:f32, i32[0..21]) -> void
    regs(edx:ptr[0x60]:f64) -> ()
    regs(esi:ptr[0x400]:layout=car, st0:f64[-1..1]) -> (eax, st0); tol=1e-9

For ``regs`` the reconstruction is ``void __cdecl fn(gp4::Regs* r)`` (see
include/gp4/gp4.h); for the others it has the stated C signature.
"""
from __future__ import annotations

import re
import math
from dataclasses import dataclass, field

GPRS = ("eax", "ebx", "ecx", "edx", "esi", "edi", "ebp")
FLAG_BITS = {"cf": 0, "pf": 2, "af": 4, "zf": 6, "sf": 7, "of": 11}
INT_TYPES = {"i8": 1, "u8": 1, "i16": 2, "u16": 2, "i32": 4, "u32": 4}
CONVS = ("cdecl", "stdcall", "thiscall", "fastcall", "regs")


class SpecError(ValueError):
    pass


@dataclass
class ArgType:
    kind: str                # int name | f32 | f64 | ptr
    lo: float | None = None
    hi: float | None = None
    size: int = 0            # ptr buffer size
    fill: str = "bytes"      # ptr fill
    nullable: bool = False   # nptr: null in about a quarter of the cases
    base: int = 0            # rec[base,stride,lo..hi]: value = base + k*stride
    stride: int = 0
    ref: str = ""            # rel[reg,stride,lo..hi]: value = <earlier input reg> + k*stride
    choices: tuple = ()      # u32{a,b,c}: drawn from the set in about 3/4 of the cases
    exclusive: bool = False  # u32{=a,b,c}: drawn only from the set (indexes, pointers)
    bias: int = 0            # ptr[N]+B: the value is buffer + B (a count kept at ptr-4, a biased base)


@dataclass
class Arg:
    t: ArgType
    reg: str | None = None   # regs convention location (eax.. / st0..)


@dataclass
class Spec:
    conv: str
    args: list[Arg]
    ret: str | None                 # compiled conventions
    outs: list[str] = field(default_factory=list)  # regs convention
    tol: float = 0.0
    cases: int = 200
    fpucw: int | None = None
    globals: str = "keep"
    emu_skip: str | None = None
    maxinsn: int = 4_000_000          # per run; raise with maxinsn=<n> for long legitimate loops
    wall: int = 0                     # emu gate wall clock (s) for very heavy routines; 0 = config default
    seed: int = 0
    raw: str = ""

    @property
    def st_inputs(self) -> list[Arg]:
        return sorted((a for a in self.args if a.reg and a.reg.startswith("st")), key=lambda a: a.reg)

    @property
    def st_outputs(self) -> list[str]:
        return sorted(o for o in self.outs if o.startswith("st"))


def _bound(text: str) -> float:
    """A range bound: decimal/float, or a hexadecimal integer such as 0x3fff or -0x80."""
    if re.fullmatch(r"[-+]?0[xX][0-9a-fA-F]+", text):
        return float(int(text, 16))
    try:
        return float(text)
    except ValueError:
        raise SpecError(f"bad range bound {text!r}") from None


def _type(tok: str) -> ArgType:
    tok = tok.strip()
    m = re.fullmatch(r"(n?)ptr\[(0x[0-9a-fA-F]+|\d+)\](?:\+(0x[0-9a-fA-F]+|\d+))?(?::(.+))?", tok)
    if m:
        nullable, bias = bool(m.group(1)), int(m.group(3), 0) if m.group(3) else 0
        m = re.fullmatch(r"n?ptr\[(0x[0-9a-fA-F]+|\d+)\](?:\+(?:0x[0-9a-fA-F]+|\d+))?(?::(.+))?", tok)
        fill = (m.group(2) or "bytes").strip()
        ranged = re.fullmatch(r"(i8|u8|i16|u16|i32|u32|f32|f64)\[([-+0-9.eExXa-fA-F]+)\.\.([-+0-9.eExXa-fA-F]+)\]", fill)
        valueset = re.fullmatch(r"(i8|u8|i16|u16|i32|u32)\{=?[-+0-9xXa-fA-F, ]+\}", fill)
        if ranged or valueset:   # every element of the buffer drawn from this typed range / value set
            _type(fill)   # validates the range or set against the element type
        elif fill.startswith("{") and fill.endswith("}"):   # inline buffer: nested pointers and typed fields
            _, fields = struct_fill(fill)
            size = int(m.group(1), 0)
            for off, count, stride, t in fields:
                if off + (count - 1) * stride + _width(t) > size:
                    raise SpecError(f"inline field at {off:#x} (x{count}) runs past the {size:#x}-byte buffer")
        elif fill.startswith("com="):
            from .com import IFACES
            if fill[4:] not in IFACES:
                raise SpecError(f"unknown COM interface {fill[4:]!r} (modelled: {', '.join(IFACES)})")
        elif not (fill in ("f32", "f64", "i32", "bytes", "zero") or fill.startswith("layout=")):
            raise SpecError(f"bad ptr fill {fill!r}")
        size = int(m.group(1), 0)
        if size <= 0:
            raise SpecError("pointer buffers must have positive size")
        if fill.startswith("layout=") and not re.fullmatch(r"layout=[A-Za-z0-9_-]+(\+[A-Za-z0-9_-]+)*", fill):
            raise SpecError("layout names must be plain file stems (layout=a or layout=a+b)")
        if bias >= size:
            raise SpecError(f"ptr[{size:#x}]+{bias:#x}: the bias must lie inside the buffer")
        return ArgType("ptr", size=size, fill=fill, nullable=nullable, bias=bias)
    m = re.fullmatch(r"rec\[(0x[0-9a-fA-F]+|\d+),(0x[0-9a-fA-F]+|\d+),(0x[0-9a-fA-F]+|\d+)\.\.(0x[0-9a-fA-F]+|\d+)\]", tok)
    if m:   # a record-aligned address into a fixed array (e.g. esi = 0x66fef0 + k*0x41c)
        base, stride, lo, hi = (int(m.group(k), 0) for k in (1, 2, 3, 4))
        if stride < 1 or lo > hi or base + hi * stride > 0xFFFFFFFF:
            raise SpecError("rec[base,stride,lo..hi] needs stride >= 1, lo <= hi and a 32-bit range")
        return ArgType("rec", float(lo), float(hi), base=base, stride=stride)
    m = re.fullmatch(r"(i8|u8|i16|u16|i32|u32)\{(=?)([-+0-9xXa-fA-F, ]+)\}", tok)
    if m:   # an integer drawn mostly ({a,b}) or only ({=a,b}) from a value set
        bits = INT_TYPES[m.group(1)] * 8
        signed = m.group(1).startswith("i")
        vals = tuple(int(v.strip(), 0) for v in m.group(3).split(",") if v.strip())
        lower = -(1 << (bits - 1)) if signed else 0
        upper = (1 << (bits - int(signed))) - 1
        if not vals or any(v < lower or v > upper for v in vals):
            raise SpecError(f"value set {tok!r} must be nonempty and fit its type")
        return ArgType(m.group(1), choices=vals, exclusive=bool(m.group(2)))
    num = r"(-?0x[0-9a-fA-F]+|-?\d+)"
    m = re.fullmatch(rf"rel\[(e[abcd]x|esi|edi|ebp),{num},{num}\.\.{num}\]", tok)
    if m:   # a register tied to an earlier input register (e.g. ebp = edi + k*0x160)
        stride, lo, hi = int(m.group(2), 0), int(m.group(3), 0), int(m.group(4), 0)
        if stride == 0 or lo > hi or abs(stride) * max(abs(lo), abs(hi)) > 0xFFFFFFFF:
            raise SpecError("rel[reg,stride,lo..hi] needs a nonzero stride, lo <= hi and a 32-bit range")
        return ArgType("rel", float(lo), float(hi), stride=stride, ref=m.group(1))
    m = re.fullmatch(r"(i8|u8|i16|u16|i32|u32|f32|f64)(?:\[([-+0-9.eExXa-fA-F]+)\.\.([-+0-9.eExXa-fA-F]+)\])?", tok)
    if not m:
        raise SpecError(f"bad type {tok!r}")
    lo = _bound(m.group(2)) if m.group(2) else None
    hi = _bound(m.group(3)) if m.group(3) else None
    if lo is not None:
        if not math.isfinite(lo) or not math.isfinite(hi) or lo > hi:
            raise SpecError("input range must be finite and ordered")
        if m.group(1) in INT_TYPES:
            bits = INT_TYPES[m.group(1)] * 8
            signed = m.group(1).startswith("i")
            lower = -(1 << (bits - 1)) if signed else 0
            upper = (1 << (bits - int(signed))) - 1
            if not lo.is_integer() or not hi.is_integer() or lo < lower or hi > upper:
                raise SpecError("integer input range must fit its type")
    return ArgType(m.group(1), lo, hi)


def _width(t: ArgType) -> int:
    return {"f32": 4, "f64": 8, "ptr": 4, "rec": 4}.get(t.kind) or INT_TYPES[t.kind]


def struct_fill(fill: str) -> tuple[str, list[tuple[int, int, int, ArgType]]]:
    """An inline buffer '{[*bytes,] off[*count[/stride]]:type, ...}' -> (background, fields).

    Every type the spec allows works for a field, including nested pointers with their own fill
    (another inline buffer, a layout, com=<Interface>) and value sets such as u32{=0x5f0758} for a
    real GP4 vtable pointer. Unlisted bytes are zero unless the first item is *bytes."""
    background, fields = "zero", []
    for part in (p.strip() for p in _split_args(fill[1:-1])):
        if not part:
            continue
        if part in ("*bytes", "*zero"):
            background = part[1:]
            continue
        m = re.fullmatch(r"(0x[0-9a-fA-F]+|\d+)(?:\*(\d+)(?:/(0x[0-9a-fA-F]+|\d+))?)?\s*:\s*(.+)", part, re.S)
        if not m:
            raise SpecError(f"bad inline field {part!r} (off[*count[/stride]]:type)")
        t = _type(m.group(4))
        if t.kind == "rel":
            raise SpecError("rel[...] is for registers, not buffer fields")
        count = int(m.group(2) or 1)
        stride = int(m.group(3), 0) if m.group(3) else _width(t)
        if not 1 <= count <= 4096 or stride < _width(t):
            raise SpecError(f"inline field {part!r}: count 1..4096 and stride >= its width")
        fields.append((int(m.group(1), 0), count, stride, t))
    if not fields:
        raise SpecError("an inline buffer needs at least one field")
    return background, fields


def parse(text: str) -> Spec:
    raw = text.strip()
    head, *opts = [p.strip() for p in raw.split(";")]
    m = re.fullmatch(r"(\w+)\s*\((.*)\)\s*->\s*(.+)", head)
    if not m:
        raise SpecError(f"cannot parse spec head {head!r}")
    conv, argstr, retstr = m.group(1), m.group(2).strip(), m.group(3).strip()
    if conv not in CONVS:
        raise SpecError(f"unknown convention {conv!r}")
    args: list[Arg] = []
    for a in filter(None, (x.strip() for x in _split_args(argstr))):
        if conv == "regs":
            reg, _, t = a.partition(":")
            reg = reg.strip()
            if reg not in GPRS and reg not in FLAG_BITS and not re.fullmatch(r"st[0-7]", reg):
                raise SpecError(f"bad register {reg!r}")
            arg_type = _type(t)
            if reg.startswith("st") and arg_type.kind not in ("f32", "f64"):
                raise SpecError("x87 inputs must be f32 or f64")
            if reg in GPRS and arg_type.kind == "f64":
                raise SpecError("a GPR cannot contain a 64-bit float")
            if reg in FLAG_BITS and (arg_type.kind not in INT_TYPES or arg_type.lo != 0 or arg_type.hi != 1):
                raise SpecError("flag inputs need an integer type with range [0..1]")
            if arg_type.kind == "rel" and (reg not in GPRS or arg_type.ref not in [x.reg for x in args]):
                raise SpecError(f"{reg}: rel[{arg_type.ref},...] must name an earlier input register")
            args.append(Arg(arg_type, reg))
        else:
            t = _type(a)
            if t.kind == "rel":
                raise SpecError("rel[...] ties registers: only the regs convention supports it")
            args.append(Arg(t))
    spec = Spec(conv=conv, args=args, ret=None, raw=raw)
    if conv == "regs":
        outs = retstr.strip("()").strip()
        spec.outs = [o.strip() for o in outs.split(",") if o.strip()]
        for o in spec.outs:
            if o not in GPRS and o not in FLAG_BITS and not re.fullmatch(r"st[0-7]", o):
                raise SpecError(f"bad output {o!r}")
        input_names = [a.reg for a in args]
        if len(input_names) != len(set(input_names)) or len(spec.outs) != len(set(spec.outs)):
            raise SpecError("duplicate input or output register")
        for names in ([a.reg for a in spec.st_inputs], spec.st_outputs):
            if names != [f"st{i}" for i in range(len(names))]:
                raise SpecError("x87 inputs and outputs must be contiguous from st0")
    else:
        if retstr not in ("void", "i32", "u32", "u8", "i8", "u16", "i16", "f32", "f64", "i64"):
            raise SpecError(f"bad return type {retstr!r}")
        spec.ret = retstr
        if conv == "thiscall" and (not args or args[0].t.kind not in ("ptr", "u32", "i32")):
            raise SpecError("thiscall needs a pointer/int 'this' as first arg")
    seen_options = set()
    for o in opts:
        if not o:
            continue
        k, _, v = o.partition("=")
        k, v = k.strip(), v.strip()
        if k in seen_options:
            raise SpecError(f"duplicate option {k!r}")
        seen_options.add(k)
        if k == "tol":
            spec.tol = float(v)
        elif k == "cases":
            spec.cases = int(v)
        elif k == "fpucw":
            spec.fpucw = int(v, 16)
        elif k == "globals":
            spec.globals = v
        elif k == "emu":
            if not v.startswith("skip"):
                raise SpecError("emu= only supports skip:<reason>")
            spec.emu_skip = v.partition(":")[2] or "unspecified"
        elif k == "maxinsn":
            spec.maxinsn = int(v)
        elif k == "wall":
            spec.wall = int(v)
            if not 0 < spec.wall <= 3600:
                raise SpecError("wall=<seconds> must be 1..3600")
        elif k == "seed":
            spec.seed = int(v)
        else:
            raise SpecError(f"unknown option {k!r}")
    if not math.isfinite(spec.tol) or spec.tol < 0:
        raise SpecError("tolerance must be finite and nonnegative")
    if spec.cases <= 0 or spec.maxinsn <= 0:
        raise SpecError("case and instruction budgets must be positive")
    if spec.globals not in ("keep", "fuzz:f32", "fuzz:f64", "fuzz:i32") and             not re.fullmatch(r"(fuzz:(f32|f64|i32)\+)?layout=[A-Za-z0-9_-]+(\+[A-Za-z0-9_-]+)*", spec.globals):
        raise SpecError("globals must be keep, fuzz:f32/f64/i32, layout=<name>[+<name>...] "
                        "or fuzz:<kind>+layout=<name>[+...] (layout fields override the fuzzed values)")
    if spec.fpucw is not None and not 0 <= spec.fpucw <= 0xFFFF:
        raise SpecError("FPU control word must fit 16 bits")
    return spec


def _split_args(s: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch in "[{":
            depth += 1
        elif ch in "]}":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    return out
