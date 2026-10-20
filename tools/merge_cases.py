"""Lead helper: attach the case bodies of a computed-jump switch to their parent function.

    python tools/merge_cases.py PARENT TABLE COUNT [--dry]

The analysis recognises `jmp [reg*4+table]` switches only. A dispatch written as
`add reg,table; jmp [reg]` (R3 menus/streams) leaves the case bodies out of the
parent, so the shape gate reports the bodies' callees and globals as extra. This
reads COUNT dword targets at TABLE, explores each target (stopping at known
function starts), and adds the targets' calls, imports and globals to the parent's
rows in `edges`, `func_imports` and `func_globals`. The verifier still decides.
"""
import argparse
import struct
import sqlite3
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from gp4re.analyze import Analyzer   # noqa: E402
from gp4re.verify import gp4_binary   # noqa: E402


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("parent"); ap.add_argument("table"); ap.add_argument("count", type=int)
    ap.add_argument("--dry", action="store_true")
    a = ap.parse_args()
    parent, table = int(a.parent, 16), int(a.table, 16)
    b = gp4_binary()
    targets = sorted(set(struct.unpack(f"<{a.count}I", b.read(table, 4 * a.count))))
    con = sqlite3.connect(ROOT / "state" / "gp4re.sqlite")
    an = Analyzer(b, log=lambda *x: None)
    an.starts = {r[0] for r in con.execute("SELECT addr FROM functions WHERE kind!='data'")} - set(targets)
    calls, imports, globals_ = set(), set(), set()
    for t in targets:
        sec = b.section_of(t)
        if not sec or not (sec.characteristics & 0x20000000):
            sys.exit(f"target {t:#x} is not in executable code: wrong table or count")
        f = an.explore(t)
        calls |= f.calls | f.tails
        imports |= f.imports
        globals_ |= f.globals_r
    globals_.discard(table)
    print(f"targets {[hex(t) for t in targets]}")
    print(f"calls {[hex(c) for c in sorted(calls)]} imports {sorted(imports)} globals {len(globals_)}")
    if a.dry:
        return
    with con:
        con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)", [(parent, c, "call") for c in calls])
        con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(parent, i) for i in imports])
        con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(parent, g) for g in globals_])
    print("merged into", hex(parent))


if __name__ == "__main__":
    main()
