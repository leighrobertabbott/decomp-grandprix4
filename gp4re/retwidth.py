"""Static check behind the narrow return types ``u8``/``i8`` (al) and ``u16``/``i16`` (ax).

A compiled ``bool``/``char`` function returns in al and leaves the upper bits of eax as
whatever its last computation put there. A spec that compares only al is sound only if no
caller reads more than al before overwriting eax. From every direct ``call`` site of the
original this walks forward (both directions of conditional branches, direct jumps, up to
64 instructions per site): a read of eax/ax/ah beyond the declared width is a violation; a
full write of eax (or a call, which clobbers it) ends the path; a ``ret`` returns the value to
the containing function's callers, which are checked the same way (up to ``depth`` levels).
Tail jumps to the function, data references (vtables, handler tables) and indirect jumps are
reported as unchecked: the gate passes them with a warning, never silently.
"""
from __future__ import annotations

import struct

import capstone
from capstone import x86_const as X

TEXT_LO = 0x401000
_FULL = {X.X86_REG_EAX}
_WORD = {X.X86_REG_AX}
_HIGH = {X.X86_REG_AH}
_LOW = {X.X86_REG_AL}


class _Walk:
    def __init__(self, binary, owner_of, width: int, depth: int = 3):
        self.b = binary
        self.owner_of = owner_of          # va -> start of the function containing it (or None)
        self.width = width                # 1 (al) or 2 (ax)
        self.depth = depth
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.md.detail = True
        text = next(s for s in binary.sections if s.executable and s.va <= TEXT_LO < s.end)
        self.text_lo, self.text = text.va, binary.read(text.va, text.end - text.va)
        data = [s for s in binary.sections if not s.executable]
        self.data = [(s.va, binary.read(s.va, s.end - s.va)) for s in data]
        self.violations: list[str] = []
        self.unchecked: list[str] = []
        self.seen_fn: set[int] = set()

    def sites(self, target: int) -> tuple[list[int], list[int]]:
        calls, jumps, t = [], [], self.text
        for i in range(len(t) - 4):
            if t[i] in (0xE8, 0xE9):
                dst = (self.text_lo + i + 5 + struct.unpack_from("<i", t, i + 1)[0]) & 0xFFFFFFFF
                if dst == target:
                    (calls if t[i] == 0xE8 else jumps).append(self.text_lo + i)
        return calls, jumps

    def data_refs(self, target: int) -> list[int]:
        pat, out = struct.pack("<I", target), []
        for va, d in self.data:
            k = d.find(pat)
            while k >= 0 and len(out) < 8:
                out.append(va + k)
                k = d.find(pat, k + 1)
        return out

    def function(self, fn: int, level: int, chain: str) -> None:
        if fn in self.seen_fn:
            return
        self.seen_fn.add(fn)
        calls, jumps = self.sites(fn)
        refs = self.data_refs(fn)
        if refs:
            self.unchecked.append(f"{chain}{fn:#x} is referenced as data at {', '.join(hex(r) for r in refs[:4])}")
        for j in jumps:
            owner = self.owner_of(j)
            if owner is None or level >= self.depth:
                self.unchecked.append(f"{chain}tail jump at {j:#x}")
            else:
                self.function(owner, level + 1, chain + f"{fn:#x} <-jmp {j:#x} | ")
        for c in calls:
            self.path(c + 5, level, chain + f"{fn:#x} <- {c:#x}")

    def path(self, start: int, level: int, chain: str) -> None:
        todo, seen, budget = [start], set(), 64
        while todo and budget > 0:
            va = todo.pop()
            while budget > 0:
                if va in seen:
                    break
                seen.add(va)
                budget -= 1
                ins = next(self.md.disasm(self.b.read(va, 16), va), None)
                if ins is None:
                    self.unchecked.append(f"{chain}: undecodable at {va:#x}")
                    break
                read, written = (set(r) for r in ins.regs_access())
                mn = ins.mnemonic
                zero_idiom = mn in ("xor", "sub") and ins.op_str in ("eax, eax", "ax, ax")
                beyond = _FULL | _WORD | _HIGH if self.width == 1 else _FULL
                if not zero_idiom and read & beyond:
                    self.violations.append(f"{chain}: {va:#x} {mn} {ins.op_str} reads beyond "
                                           f"{'al' if self.width == 1 else 'ax'}")
                    return
                if mn == "call" or zero_idiom or written & _FULL:
                    break                                  # eax overwritten: this path is fine
                if mn in ("ret", "retf"):
                    fn = self.owner_of(va)
                    if fn is None or level >= self.depth:
                        self.unchecked.append(f"{chain}: value returned at {va:#x} (depth limit or unknown function)")
                    else:
                        self.function(fn, level + 1, chain + f" | ret {va:#x} of ")
                    break
                if mn == "jmp":
                    if ins.op_str.startswith("0x"):
                        va = int(ins.op_str, 16)
                        continue
                    self.unchecked.append(f"{chain}: indirect jmp at {va:#x}")
                    break
                if mn.startswith("j") or mn.startswith("loop"):
                    if ins.op_str.startswith("0x"):
                        todo.append(int(ins.op_str, 16))
                va += ins.size
        if budget <= 0:
            self.unchecked.append(f"{chain}: no write of eax within 64 instructions")


def check(binary, target: int, width: int, owner_of) -> tuple[list[str], list[str]]:
    """Return (violations, unchecked) for a function whose result is declared width bytes wide."""
    w = _Walk(binary, owner_of, width)
    w.function(target, 0, "")
    return w.violations, w.unchecked
