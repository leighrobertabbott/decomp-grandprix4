"""Calling-convention / test-input spec carried by every reconstruction.

Grammar (whitespace-insensitive)::

    spec    := conv "(" [arg ("," arg)*] ")" "->" ret [";" opt]*
    conv    := cdecl | stdcall | thiscall | fastcall | regs
    arg     := type                       # stack/ecx/edx args for compiled conventions
             | reg ":" type               # regs convention: eax..ebp, st0..st7, arithmetic flags
    type    := i32 | u32 | i16 | u16 | i8 | u8 | f32 | f64 [ "[" lo ".." hi "]" ]
             | ptr "[" size "]" [":" fill]
    fill    := f32 | f64 | i32 | bytes | zero | layout=<name>
    ret     := void | i32 | u32 | f32 | f64 | i64 | "(" out ("," out)* ")" | "()"
    out     := eax|ebx|ecx|edx|esi|edi|ebp|st0..st7|cf|pf|af|zf|sf|of
    opt     := tol=<float> | cases=<n> | fpucw=<hex> | globals=keep|fuzz:<f32|f64|i32>|layout=<name>
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
    maxinsn: int = 20_000_000
    seed: int = 0
    raw: str = ""

    @property
    def st_inputs(self) -> list[Arg]:
        return sorted((a for a in self.args if a.reg and a.reg.startswith("st")), key=lambda a: a.reg)

    @property
    def st_outputs(self) -> list[str]:
        return sorted(o for o in self.outs if o.startswith("st"))


def _type(tok: str) -> ArgType:
    tok = tok.strip()
    m = re.fullmatch(r"ptr\[(0x[0-9a-fA-F]+|\d+)\](?::(.+))?", tok)
    if m:
        fill = (m.group(2) or "bytes").strip()
        if not (fill in ("f32", "f64", "i32", "bytes", "zero") or fill.startswith("layout=")):
            raise SpecError(f"bad ptr fill {fill!r}")
        size = int(m.group(1), 0)
        if size <= 0:
            raise SpecError("pointer buffers must have positive size")
        if fill.startswith("layout=") and not re.fullmatch(r"layout=[A-Za-z0-9_-]+", fill):
            raise SpecError("layout name must be a plain file stem")
        return ArgType("ptr", size=size, fill=fill)
    m = re.fullmatch(r"(i8|u8|i16|u16|i32|u32|f32|f64)(?:\[([-+0-9.e]+)\.\.([-+0-9.e]+)\])?", tok)
    if not m:
        raise SpecError(f"bad type {tok!r}")
    lo = float(m.group(2)) if m.group(2) else None
    hi = float(m.group(3)) if m.group(3) else None
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
            args.append(Arg(arg_type, reg))
        else:
            args.append(Arg(_type(a)))
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
        if retstr not in ("void", "i32", "u32", "f32", "f64", "i64"):
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
        elif k == "seed":
            spec.seed = int(v)
        else:
            raise SpecError(f"unknown option {k!r}")
    if not math.isfinite(spec.tol) or spec.tol < 0:
        raise SpecError("tolerance must be finite and nonnegative")
    if spec.cases <= 0 or spec.maxinsn <= 0:
        raise SpecError("case and instruction budgets must be positive")
    if spec.globals not in ("keep", "fuzz:f32", "fuzz:f64", "fuzz:i32") and not re.fullmatch(r"layout=[A-Za-z0-9_-]+", spec.globals):
        raise SpecError("globals must be keep, fuzz:f32/f64/i32, or layout=<name>")
    if spec.fpucw is not None and not 0 <= spec.fpucw <= 0xFFFF:
        raise SpecError("FPU control word must fit 16 bits")
    return spec


def _split_args(s: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "[":
            depth += 1
        elif ch == "]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    return out
