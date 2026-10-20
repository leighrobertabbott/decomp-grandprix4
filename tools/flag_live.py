"""Lead helper: are the flags a function returns ever read?

    python tools/flag_live.py ADDR [--depth 4]

Evidence for specs/flag_exempt.json. From every direct `call`/`jmp ADDR` site the scan walks
forward: a flag read (jcc, adc/sbb, setcc, cmovcc, pushf, lahf, rcl/rcr, loopcc) is LIVE; an
instruction that writes the arithmetic flags (or a call, whose callee writes them) ends the
path as dead; mov/push/pop/lea/pushal/popal/xchg and other flag-neutral instructions are
skipped; a direct jmp is followed; `ret` continues at every call site of the containing
function (up to --depth levels). Indirect jumps and functions referenced as data (handler
tables) are reported, never assumed dead. The verifier still decides; this only gathers
the static evidence.
"""
import argparse
import re
import sqlite3
import struct
import sys
from pathlib import Path

import capstone

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from gp4re.verify import gp4_binary   # noqa: E402

READ = re.compile(r"^(j(?!mp)|adc|sbb|set|cmov|pushf|lahf|rcl|rcr|loope|loopne|loopz|loopnz|into|salc)")
WRITE = re.compile(r"^(add|sub|and|or|xor|cmp|test|inc|dec|neg|shl|shr|sar|sal|rol|ror|shld|shrd|imul|mul|"
                   r"div|idiv|bt|bts|btr|btc|bsf|bsr|popf|sahf|clc|stc|cmc|scas|cmps|daa|das|aaa|aas|cmpxchg|xadd)")


class Scan:
    def __init__(self, depth: int):
        self.b = gp4_binary()
        self.con = sqlite3.connect(ROOT / "state" / "gp4re.sqlite")
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.text = self.b.read(0x401000, 2025526)
        self.data = self.b.read(0x601000, 0x547000)
        self.depth = depth
        self.live, self.unknown, self.dead = [], [], 0
        self.unreferenced: list[str] = []
        self.seen: set[int] = set()

    def sites(self, tgt: int) -> list[int]:
        t, out = self.text, []
        for i in range(len(t) - 5):
            if t[i] in (0xE8, 0xE9) and (0x401000 + i + 5 + struct.unpack_from("<i", t, i + 1)[0]) & 0xFFFFFFFF == tgt:
                out.append(0x401000 + i)
        return out

    def data_refs(self, tgt: int) -> list[int]:
        d = self.data
        return [0x601000 + i for i in range(len(d) - 3) if int.from_bytes(d[i:i + 4], "little") == tgt]

    def owner(self, va: int) -> int | None:
        r = self.con.execute("SELECT addr FROM functions WHERE addr<=? AND end_addr>? AND kind!='data' "
                             "ORDER BY addr DESC LIMIT 1", (va, va)).fetchone()
        return r[0] if r else None

    def callers(self, fn: int, level: int, chain: str) -> None:
        refs = self.data_refs(fn)
        if refs:
            self.unknown.append(f"{chain}: {fn:#x} is referenced as data at {', '.join(hex(r) for r in refs[:4])}")
        sites = self.sites(fn)
        if not sites and not refs:
            self.unreferenced.append(f"{chain}: {fn:#x} has no call/jmp sites and no data references")
        for s in sites:
            if self.text[s - 0x401000] == 0xE9:            # tail jump: the flags leave with fn's own ret
                self.callers(self.owner(s) or s, level, chain + f" <-jmp {s:#x}")
            else:
                self.walk(s + 5, level, chain + f" <- {s:#x}")

    def walk(self, va: int, level: int, chain: str) -> None:
        for _ in range(64):
            if va in self.seen:
                self.dead += 1
                return
            self.seen.add(va)
            ins = next(self.md.disasm(self.b.read(va, 16), va), None)
            if ins is None:
                self.unknown.append(f"{chain}: undecodable at {va:#x}")
                return
            mn = ins.mnemonic
            if READ.match(mn):
                self.live.append(f"{chain}: {va:#x} {mn} {ins.op_str}")
                return
            if WRITE.match(mn) or mn == "call":
                self.dead += 1
                return
            if mn == "jmp":
                if ins.op_str.startswith("0x"):
                    va = int(ins.op_str, 16)
                    continue
                self.unknown.append(f"{chain}: indirect jmp at {va:#x}")
                return
            if mn in ("ret", "retf"):
                fn = self.owner(va)
                if fn is None:
                    self.unknown.append(f"{chain}: ret at {va:#x} outside a known function")
                elif level >= self.depth:
                    self.unknown.append(f"{chain}: depth limit at ret {va:#x} (function {fn:#x})")
                else:
                    self.callers(fn, level + 1, chain + f" | ret of {fn:#x}")
                return
            va += ins.size
        self.unknown.append(f"{chain}: no flag writer within 64 instructions of {va:#x}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("addr")
    ap.add_argument("--depth", type=int, default=4)
    a = ap.parse_args()
    tgt = int(a.addr, 16)
    s = Scan(a.depth)
    s.callers(tgt, 0, f"{tgt:#x}")
    print(f"{tgt:#010x}: {len(s.live)} live read(s), {len(s.unknown)} unknown, {s.dead} dead path(s), "
          f"{len(s.unreferenced)} path(s) ending in an unreferenced function")
    for line in s.unreferenced[:10]:
        print("  END ", line)
    for line in s.live[:20]:
        print("  LIVE", line)
    for line in s.unknown[:20]:
        print("  ?   ", line)


if __name__ == "__main__":
    main()
