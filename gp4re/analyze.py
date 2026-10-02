"""Static function discovery for 32-bit MSVC-era binaries (``gp4re analyze``).

No IDA/Ghidra required: capstone recursive descent seeded from the entry point,
code pointers in data, immediate code pointers and call targets, followed by a
gap-filling pass. Produces, per function: extent, call/tail edges, imports,
globals, strings, jump tables, frame/stack facts and a *calling-style* guess.

The calling-style guess matters for GP4: Geoff Crammond wrote the physics and AI
in hand-written assembly, which shows up as functions that consume registers,
flags or x87 stack values that a VC6-compiled function never would. Those are
tagged ``asm-like`` and routed to the semantic (emulator-differential) track
instead of the matching track.

If Ghidra is installed, ``gp4re ghidra`` later adds pseudocode and types on top
of these function boundaries; it does not replace them.
"""
from __future__ import annotations

import hashlib
import re
import time
from dataclasses import dataclass, field

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from .binary import Binary

TERMINATORS = {"ret", "retf", "iret", "iretd", "hlt", "int3", "ud2"}
COND_EXTRA = {"loop", "loope", "loopne", "jecxz", "jcxz"}
NORETURN_IMPORTS = {"kernel32.dll!ExitProcess", "kernel32.dll!ExitThread",
                    "kernel32.dll!FatalExit", "kernel32.dll!FatalAppExitA"}
PAD = {0xCC, 0x90}
MULTI_NOPS = sorted((bytes.fromhex(h) for h in (
    "8da42400000000", "8d9b00000000", "8da40000000000", "8d642400", "8d4900",
    "8d4000", "8d7600", "8d3f", "8d36", "8bc0", "8bff", "8bc9", "8bd2", "8bdb", "8bf6", "87db",
)), key=len, reverse=True)
X87_LOADS = ("fld", "fild", "fldz", "fld1", "fldpi", "fldl2e", "fldln2", "fldlg2", "fldl2t",
             "fldcw", "fnstcw", "fstcw", "fninit", "finit", "fnclex", "fclex", "fwait", "wait",
             "fnstsw", "fstsw", "fnsave", "frstor", "fxsave", "fxrstor", "fnstenv", "fldenv", "fbld")
REG32 = {"al": "eax", "ah": "eax", "ax": "eax", "eax": "eax", "bl": "ebx", "bh": "ebx", "bx": "ebx",
         "ebx": "ebx", "cl": "ecx", "ch": "ecx", "cx": "ecx", "ecx": "ecx", "dl": "edx", "dh": "edx",
         "dx": "edx", "edx": "edx", "si": "esi", "esi": "esi", "di": "edi", "edi": "edi",
         "bp": "ebp", "ebp": "ebp", "sp": "esp", "esp": "esp"}
ASM_INPUT_REGS = {"eax", "ebx", "edx", "esi", "edi", "ebp"}

_MEM = re.compile(r"\[([^\]]+)\]")
_NUM = re.compile(r"^(0x[0-9a-f]+|\d+)$")


def _int(tok: str) -> int | None:
    tok = tok.strip()
    if _NUM.match(tok):
        return int(tok, 0)
    return None


def parse_mem(inner: str) -> tuple[str | None, str | None, int, int]:
    """'eax*4 + 0x4a1000' -> (base, index, scale, disp)."""
    base = index = None
    scale, disp = 1, 0
    for sign, term in re.findall(r"([+-]?)\s*([^+-]+)", inner.replace(" ", "")):
        term = term.strip()
        if "*" in term:
            r, s = term.split("*")
            index, scale = r, int(s, 0)
        elif _int(term) is not None:
            disp += -_int(term) if sign == "-" else _int(term)
        elif term in REG32:
            if base is None:
                base = term
            else:
                index = term
    return base, index, scale, disp & 0xFFFFFFFF


def split_operands(op_str: str) -> list[str]:
    return [o.strip() for o in op_str.split(",")] if op_str else []


@dataclass(slots=True)
class Insn:
    va: int
    size: int
    mn: str
    ops: str


@dataclass
class Func:
    start: int
    insns: dict[int, Insn] = field(default_factory=dict)
    calls: set[int] = field(default_factory=set)
    tails: set[int] = field(default_factory=set)
    imports: set[str] = field(default_factory=set)
    globals_r: set[int] = field(default_factory=set)
    code_ptrs: set[int] = field(default_factory=set)
    jtables: list[tuple[int, int]] = field(default_factory=list)
    byte_tables: set[int] = field(default_factory=set)
    ret_pops: set[int] = field(default_factory=set)
    n_cond: int = 0
    invalid: bool = False
    indirect_calls: int = 0
    indirect_jumps: int = 0
    source: str = ""

    @property
    def end(self) -> int:
        return max(i.va + i.size for i in self.insns.values()) if self.insns else self.start

    @property
    def size(self) -> int:
        return sum(i.size for i in self.insns.values())


class Analyzer:
    def __init__(self, b: Binary, log=print):
        self.b = b
        self.log = log
        self.md = Cs(CS_ARCH_X86, CS_MODE_32)
        self.mdd = Cs(CS_ARCH_X86, CS_MODE_32)
        self.mdd.detail = True
        code = b.code_sections()
        if not code:
            raise SystemExit("no in-scope executable section")
        self.text = code[0]
        self.t0, self.t1 = self.text.va, self.text.va + self.text.vsize
        self.cache: dict[int, Insn | None] = {}
        self.starts: set[int] = set()
        self.noreturn: set[int] = set()
        self.funcs: dict[int, Func] = {}
        self.cov = bytearray(self.t1 - self.t0)  # 0 unknown, 1 code, 2 data-in-text, 3 padding
        self.data_ranges = [(s.va, s.end) for s in b.data_sections()]

    # ------------------------------------------------------------- primitives
    def in_text(self, va: int) -> bool:
        return self.t0 <= va < self.t1

    def in_data(self, va: int) -> bool:
        return any(a <= va < e for a, e in self.data_ranges)

    def decode(self, va: int) -> Insn | None:
        if va in self.cache:
            return self.cache[va]
        if not self.in_text(va):
            return None
        off = va - self.b.base
        chunk = bytes(self.b.image[off: min(off + 512, self.t1 - self.b.base)])
        first = None
        for a, s, m, o in self.md.disasm_lite(chunk, va):
            ins = Insn(a, s, m, o)
            if a not in self.cache:
                self.cache[a] = ins
            if first is None:
                first = ins
            if m in TERMINATORS or m == "jmp":
                break
        if first is None:
            self.cache[va] = None
        return first

    # ------------------------------------------------------------- descent
    def explore(self, start: int, source: str = "") -> Func:
        f = Func(start=start, source=source)
        work = [start]
        last_iat: dict[str, str] = {}
        while work:
            va = work.pop()
            last_iat.clear()
            while True:
                if va in f.insns:
                    break
                if va != start and va in self.starts:
                    f.tails.add(va)  # fell through / branched into another function
                    break
                ins = self.decode(va)
                if ins is None:
                    f.invalid = True
                    break
                f.insns[va] = ins
                mn, ops = ins.mn, ins.ops
                nxt = va + ins.size
                self._refs(f, ins, last_iat)
                if mn == "call":
                    t = _int(ops)
                    if t is not None:
                        if self.in_text(t):
                            f.calls.add(t)
                            if t in self.noreturn:
                                break
                    else:
                        f.indirect_calls += 1
                        name = self._iat_target(ops, last_iat)
                        if name:
                            f.imports.add(name)
                            if name in NORETURN_IMPORTS:
                                break
                elif mn == "jmp":
                    t = _int(ops)
                    if t is not None:
                        if t != start and (t in self.starts or not self.in_text(t)):
                            f.tails.add(t)
                        elif self.in_text(t) and t not in f.insns:
                            work.append(t)
                    else:
                        name = self._iat_target(ops, last_iat)
                        if name:
                            f.imports.add(name)
                        else:
                            entries = self._jump_table(f, ins)
                            if entries:
                                work.extend(e for e in entries if e not in f.insns)
                            else:
                                f.indirect_jumps += 1
                    break
                elif (mn.startswith("j") or mn in COND_EXTRA):
                    f.n_cond += 1
                    t = _int(ops)
                    if t is not None and self.in_text(t) and t not in f.insns:
                        if t in self.starts and t != start:
                            f.tails.add(t)
                        else:
                            work.append(t)
                elif mn in TERMINATORS:
                    if mn in ("ret", "retf"):
                        f.ret_pops.add(_int(ops) or 0)
                    break
                va = nxt
        return f

    def _iat_target(self, ops: str, last_iat: dict[str, str]) -> str | None:
        m = _MEM.search(ops)
        if m:
            base, index, _, disp = parse_mem(m.group(1))
            if base is None and index is None and disp in self.b.imports:
                return self.b.imports[disp]
            return None
        reg = ops.strip()
        return last_iat.get(REG32.get(reg, reg))

    def _refs(self, f: Func, ins: Insn, last_iat: dict[str, str]) -> None:
        ops = split_operands(ins.ops)
        for i, o in enumerate(ops):
            m = _MEM.search(o)
            if m:
                base, index, scale, disp = parse_mem(m.group(1))
                if disp in self.b.imports and base is None and index is None:
                    if ins.mn == "mov" and i == 1 and ops[0] in REG32:
                        last_iat[REG32[ops[0]]] = self.b.imports[disp]
                    continue
                if disp and self.in_data(disp):
                    f.globals_r.add(disp)
                elif disp and self.in_text(disp) and ins.mn in ("mov", "movzx") and "byte ptr" in o:
                    f.byte_tables.add(disp)
            else:
                v = _int(o)
                if v is None or ins.mn.startswith("j") or ins.mn == "call":
                    continue
                if self.in_data(v):
                    f.globals_r.add(v)
                elif self.in_text(v) and v > self.t0:
                    f.code_ptrs.add(v)

    def _jump_table(self, f: Func, ins: Insn) -> list[int]:
        m = _MEM.search(ins.ops)
        if not m:
            return []
        base, index, scale, disp = parse_mem(m.group(1))
        if not (index and scale == 4 and base is None and self.in_text(disp)):
            return []
        limit = self._table_bound(f, ins, index)
        entries = []
        for k in range(limit or 1024):
            a = disp + 4 * k
            if not self.in_text(a) or (k and a in self.starts):
                break
            e = self.b.u32(a)
            if not self.in_text(e):
                break
            entries.append(e)
        if entries:
            f.jtables.append((disp, len(entries)))
        return entries

    def _table_bound(self, f: Func, jmp: Insn, index_reg: str) -> int | None:
        # look back a few instructions for "cmp <idx>, N ; ja default"
        va = jmp.va
        prev = sorted(a for a in f.insns if a < va)[-6:]
        for a in reversed(prev):
            i = f.insns[a]
            if i.mn == "cmp":
                ops = split_operands(i.ops)
                if len(ops) == 2 and REG32.get(ops[0]) == REG32.get(index_reg) and _int(ops[1]) is not None:
                    return _int(ops[1]) + 1
        return None

    # ------------------------------------------------------------- seeds
    def sweep_calls(self) -> dict[int, int]:
        """Linear sweep of .text collecting direct call targets (weak seeds)."""
        targets: dict[int, int] = {}
        off0, end = self.t0 - self.b.base, self.t1 - self.b.base
        code = bytes(self.b.image[off0:end])
        pos = 0
        while pos < len(code):
            last = pos
            for a, s, m, o in self.md.disasm_lite(code[pos:], self.t0 + pos):
                last = a - self.t0 + s
                if m == "call":
                    t = _int(o)
                    if t is not None and self.in_text(t):
                        targets[t] = targets.get(t, 0) + 1
            pos = last + 1 if last == pos else last
        return targets

    def data_code_pointers(self) -> set[int]:
        out = set()
        for s in self.b.data_sections():
            raw = self.b.read(s.va, s.raw_size)
            for o in range(0, len(raw) - 3, 4):
                v = int.from_bytes(raw[o:o + 4], "little")
                if self.t0 < v < self.t1:
                    out.add(v)
        return out

    # ------------------------------------------------------------- coverage
    def _mark(self, f: Func) -> None:
        for i in f.insns.values():
            o = i.va - self.t0
            self.cov[o: o + i.size] = b"\x01" * i.size
        for t, n in f.jtables:
            o = t - self.t0
            if 0 <= o < len(self.cov):
                self.cov[o: o + 4 * n] = b"\x02" * min(4 * n, len(self.cov) - o)

    def covered(self, va: int) -> bool:
        return self.in_text(va) and self.cov[va - self.t0] != 0

    def _add(self, start: int, source: str, queue: list[tuple[int, str]]) -> None:
        if start not in self.starts:
            self.starts.add(start)
            queue.append((start, source))

    def _closure(self, queue: list[tuple[int, str]]) -> int:
        n = 0
        while queue:
            s, src = queue.pop()
            if s in self.funcs:
                continue
            f = self.explore(s, src)
            self.funcs[s] = f
            self._mark(f)
            n += 1
            for c in f.calls:
                self._add(c, "call", queue)
            for c in f.code_ptrs:
                if not self.covered(c) and self.decode(c):
                    self._add(c, "imm-ptr", queue)
        return n

    def _plausible_start(self, va: int) -> bool:
        if self.covered(va) or self.decode(va) is None:
            return False
        trial = self.explore(va)
        if trial.invalid or len(trial.insns) < 2:
            return False
        if not (trial.ret_pops or trial.tails or any(i.mn == "jmp" for i in trial.insns.values())):
            return False
        for i in trial.insns.values():
            o = i.va - self.t0
            if any(self.cov[o: o + i.size]):
                return False
        return True

    def _skip_padding(self, va: int, end: int) -> int:
        while va < end:
            b = self.b.image[va - self.b.base]
            if b in PAD:
                self.cov[va - self.t0] = 3
                va += 1
                continue
            for nop in MULTI_NOPS:
                if self.b.read(va, len(nop)) == nop and va + len(nop) <= end:
                    self.cov[va - self.t0: va - self.t0 + len(nop)] = b"\x03" * len(nop)
                    va += len(nop)
                    break
            else:
                return va
        return va

    def gaps(self):
        o, n = 0, len(self.cov)
        while o < n:
            if self.cov[o] == 0:
                s = o
                while o < n and self.cov[o] == 0:
                    o += 1
                yield self.t0 + s, self.t0 + o
            else:
                o += 1

    def fill_gaps(self, byte_tables: set[int]) -> int:
        added = 0
        queue: list[tuple[int, str]] = []
        for g0, g1 in list(self.gaps()):
            va = g0
            while va < g1:
                va = self._skip_padding(va, g1)
                if va >= g1:
                    break
                if va in byte_tables:
                    nxt = va
                    while nxt < g1 and self.cov[nxt - self.t0] == 0:
                        nxt += 1
                    self.cov[va - self.t0: nxt - self.t0] = b"\x02" * (nxt - va)
                    va = nxt
                    continue
                if va % 4 == 0 and va + 8 <= g1 and all(self.in_text(self.b.u32(va + 4 * k)) for k in range(2)):
                    while va + 4 <= g1 and self.in_text(self.b.u32(va)):
                        self.cov[va - self.t0: va - self.t0 + 4] = b"\x02\x02\x02\x02"
                        va += 4
                    continue
                if self._plausible_start(va):
                    self._add(va, "gap", queue)
                    added += self._closure(queue)
                    while va < g1 and self.cov[va - self.t0] != 0:
                        va += 1
                    continue
                self.cov[va - self.t0] = 2
                va += 1
        return added

    # ------------------------------------------------------------- driver
    def run(self) -> dict[int, Func]:
        t = time.time()
        queue: list[tuple[int, str]] = []
        if self.in_text(self.b.entry):
            self._add(self.b.entry, "entry", queue)
        n = self._closure(queue)
        self.log(f"  entry closure: {n} functions")

        ptrs = self.data_code_pointers()
        for p in sorted(ptrs):
            if not self.covered(p) and self.decode(p):
                self._add(p, "data-ptr", queue)
        n = self._closure(queue)
        self.log(f"  data code-pointer seeds: +{n} ({len(ptrs)} candidate pointers)")

        sweep = self.sweep_calls()
        for tgt, cnt in sorted(sweep.items()):
            if not self.covered(tgt) and (cnt >= 2 or self._plausible_start(tgt)):
                self._add(tgt, "sweep-call", queue)
        n = self._closure(queue)
        self.log(f"  linear-sweep call targets: +{n}")

        byte_tables = set().union(*(f.byte_tables for f in self.funcs.values())) if self.funcs else set()
        for rnd in range(4):
            n = self.fill_gaps(byte_tables)
            self.log(f"  gap fill round {rnd + 1}: +{n}")
            if not n:
                break

        # pass 2: recompute bodies with full start set + no-return knowledge
        for _ in range(2):
            self.noreturn = {s for s, f in self.funcs.items()
                             if not f.ret_pops and not f.tails and not f.indirect_jumps
                             and not f.jtables and not f.invalid and len(f.insns) > 1}
            for s in list(self.funcs):
                self.funcs[s] = self.explore(s, self.funcs[s].source)
        self.log(f"  re-explored with {len(self.noreturn)} no-return functions; "
                 f"{len(self.funcs)} functions in {time.time() - t:.1f}s")
        return self.funcs

    # ------------------------------------------------------------- per-function facts
    def entry_facts(self, f: Func) -> dict:
        """Registers / flags / x87 values consumed before being produced."""
        regs_in: set[str] = set()
        written: set[str] = set()
        flags_in = fpu_in = False
        fpu_seen = False
        code = self.b.read(f.start, 256)
        for n, ins in enumerate(self.mdd.disasm(code, f.start)):
            if n >= 40:
                break
            try:
                r, w = ins.regs_access()
            except Exception:
                break
            rn = {ins.reg_name(x) for x in r}
            wn = {ins.reg_name(x) for x in w}
            mn = ins.mnemonic
            ops = split_operands(ins.op_str)
            if mn in ("pushal", "pushad", "pusha", "pushfd", "pushf", "pushal"):
                continue  # whole-register / flags save, not a consumption
            if mn == "push" and ops and ops[0] in REG32:
                # callee-save, or VC6's "push ecx" 4-byte local reservation
                rn.discard(ops[0])
            if mn in ("xor", "sub", "sbb") and len(ops) == 2 and ops[0] == ops[1]:
                rn.discard(ops[0])
                if mn == "sbb":
                    rn.discard("eflags")
            if "eflags" in rn and "eflags" not in written:
                flags_in = True
            for reg in rn:
                full = REG32.get(reg)
                if full and full not in written and full != "esp":
                    regs_in.add(full)
            for reg in wn:
                written.add(REG32.get(reg, reg))
            if mn.startswith("f") and not fpu_seen:
                fpu_seen = True
                if not mn.startswith(X87_LOADS):
                    fpu_in = True
            if mn in TERMINATORS or mn == "call" or mn.startswith("j") or mn in COND_EXTRA:
                break
        return {"regs_in": sorted(regs_in), "flags_in": flags_in, "fpu_in": fpu_in}

    def summarize(self, f: Func) -> dict:
        ins = [f.insns[a] for a in sorted(f.insns)]
        x87 = sum(1 for i in ins if i.mn.startswith("f"))
        mmx = sum(1 for i in ins if re.search(r"\bmm\d\b", i.ops))
        sse = sum(1 for i in ins if "xmm" in i.ops)
        has_frame = len(ins) >= 2 and ins[0].mn == "push" and ins[0].ops == "ebp" \
            and ins[1].mn == "mov" and ins[1].ops == "ebp, esp"
        frame_size = 0
        for i in ins[:6]:
            if i.mn == "sub" and i.ops.startswith("esp, "):
                frame_size = _int(i.ops.split(", ")[1]) or 0
                break
        args_max = 0
        if has_frame:
            for i in ins:
                for m in _MEM.finditer(i.ops):
                    base, index, _, disp = parse_mem(m.group(1))
                    if base == "ebp" and index is None and 8 <= disp < 0x200:
                        args_max = max(args_max, (disp - 8) // 4 + 1)
        facts = self.entry_facts(f)
        regs = set(facts["regs_in"])
        uses_parent_frame = any(re.search(r"\[ebp - ", i.ops) for i in ins[:8])
        if len(ins) <= 2 and ins and ins[-1].mn == "jmp":
            style = "thunk"
        elif regs == {"ebp"} and not has_frame and uses_parent_frame and len(ins) <= 24:
            style = "eh-funclet"  # VC6 C++ unwind funclet running on its parent's frame
        elif regs & ASM_INPUT_REGS or facts["flags_in"] or facts["fpu_in"]:
            style = "asm-like"
        elif "ecx" in regs and "edx" in regs:
            style = "fastcall"
        elif "ecx" in regs:
            style = "thiscall"
        else:
            style = "std"
        pops = sorted(f.ret_pops)
        # normalised byte hash: identical code modulo absolute/relative addresses ("twins")
        h = hashlib.sha1()
        for i in ins:
            raw = bytearray(self.b.read(i.va, i.size))
            if (i.mn == "call" or i.mn.startswith("j")) and _int(i.ops) is not None and i.size >= 5:
                raw[-4:] = b"\0\0\0\0"
            for k in range(len(raw) - 3):
                if self.b.in_image(int.from_bytes(raw[k:k + 4], "little")):
                    raw[k:k + 4] = b"\0\0\0\0"
            h.update(raw)
        # registers used as pointers (base of [reg+disp]) and the largest displacement seen:
        # sizes pointer inputs of register-convention routines
        reg_bases: dict[str, int] = {}
        for i in ins:
            for m in _MEM.finditer(i.ops):
                base, index, _, disp = parse_mem(m.group(1))
                if base and base not in ("esp", "ebp") and disp < 0x10000:
                    full = REG32[base]
                    reg_bases[full] = max(reg_bases.get(full, 0), disp + 8)
        return {
            "norm_hash": h.hexdigest()[:16], "reg_bases": reg_bases,
            "addr": f.start, "end": f.end, "size": f.size, "n_insn": len(ins),
            "n_cond": f.n_cond, "x87": x87, "mmx": mmx, "sse": sse,
            "has_frame": has_frame, "frame_size": frame_size, "args_est": args_max,
            "ret_pop": pops[0] if len(pops) == 1 else (None if not pops else -1),
            "indirect_calls": f.indirect_calls, "indirect_jumps": f.indirect_jumps,
            "invalid": f.invalid, "source": f.source, "style": style, **facts,
            "calls": sorted(f.calls), "tails": sorted(f.tails), "imports": sorted(f.imports),
            "globals": sorted(f.globals_r), "jtables": f.jtables,
            "strings": {g: s for g in sorted(f.globals_r) if (s := self.b.cstring(g))},
        }


def analyze(b: Binary, log=print) -> tuple[list[dict], dict]:
    a = Analyzer(b, log=log)
    funcs = a.run()
    rows = [a.summarize(f) for f in sorted(funcs.values(), key=lambda f: f.start)]
    cov = a.cov
    stats = {
        "functions": len(rows),
        "text_bytes": len(cov),
        "code_bytes": cov.count(1),
        "data_in_text_bytes": cov.count(2),
        "padding_bytes": cov.count(3),
        "unknown_bytes": cov.count(0),
        "noreturn": sorted(a.noreturn),
    }
    return rows, stats
