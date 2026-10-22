"""Lead helper: re-explore functions whose analysis stopped at a start that is now excluded data.

    python tools/reextent.py [--apply] [--state todo,deferred] [ADDR ...]
    python tools/reextent.py --force --apply ADDR ...   # after tools/add_starts.py registered a start
                                                      # inside them (a shared tail): they shrink

`gp4re analyze` explores every function up to the next known start. When a start is later
excluded as data (a false start inside another function: mid-instruction bytes, string tails,
u16 pairs, stale padding), the function that contained it keeps the truncated extent, edges,
imports and globals of that first analysis, so the shape gate compares the reconstruction with
a fragment of the original (0x465650 was recorded as 3 bytes; its body runs to 0x46589a).

Each candidate is explored again with the current starts (kind != 'data'). A function is
updated only when the new exploration is larger AND covers the address of an excluded data
row, i.e. the difference is explained by a removed false start. Rows are rewritten exactly as
tools/add_starts.py writes them (functions summary columns, edges, func_imports, func_globals,
jtables); the ledger is untouched. Without --apply nothing is written. Rerunning
`gp4re analyze` rebuilds these tables from scratch (and already ignores excluded starts only if
they are excluded before it runs) - rerun this tool after any re-analysis.
"""
import argparse
import json
import sqlite3
import sys
import time
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from gp4re.analyze import Analyzer           # noqa: E402
from gp4re.db import difficulty              # noqa: E402
from gp4re.verify import gp4_binary          # noqa: E402


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("addrs", nargs="*")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--state", default="todo,deferred,claimed,accepted,accepted-runtime")
    ap.add_argument("--force", action="store_true", help="rewrite the given ADDRs whatever changed")
    a = ap.parse_args()
    b = gp4_binary()
    con = sqlite3.connect(ROOT / "state" / "gp4re.sqlite")
    data_starts = {r[0] for r in con.execute("SELECT addr FROM functions WHERE kind='data'")}
    an = Analyzer(b, log=lambda *x: None)
    an.starts = {r[0] for r in con.execute("SELECT addr FROM functions WHERE kind!='data'")}
    states = tuple(a.state.split(","))
    q = ("SELECT f.addr, f.size, f.end_addr, l.state FROM functions f JOIN ledger l USING(addr) "
         f"WHERE f.kind!='data' AND l.state IN ({','.join('?' * len(states))})")
    rows = con.execute(q, states).fetchall()
    if a.addrs:
        want = {int(x, 16) for x in a.addrs}
        rows = [r for r in rows if r[0] in want]
    changed = []
    for addr, size, end, state in rows:
        f = an.explore(addr, "lead")
        if addr not in f.insns:
            continue
        covered = [d for d in data_starts if any(i.va <= d < i.va + i.size for i in f.insns.values())] \
            if f.end > end or len(f.insns) and sum(i.size for i in f.insns.values()) > size else []
        new_size = sum(i.size for i in f.insns.values())
        if a.force and a.addrs and new_size != size:
            changed.append((addr, state, size, new_size, sorted(covered), an.summarize(f)))
        elif new_size > size and covered:
            changed.append((addr, state, size, new_size, sorted(covered), an.summarize(f)))
    print(f"{len(changed)} of {len(rows)} functions grow once excluded data starts stop truncating them")
    print("by state:", dict(Counter(c[1] for c in changed)))
    for addr, state, old, new, cov, _ in changed[:40]:
        print(f"  {addr:#010x} {state:16} size {old} -> {new}  past data rows {', '.join(hex(d) for d in cov[:4])}")
    if not a.apply:
        return
    now = time.time()
    with con:
        for addr, state, old, new, cov, r in changed:
            con.execute(
                "UPDATE functions SET end_addr=?, size=?, n_insn=?, n_cond=?, x87=?, mmx=?, sse=?, has_frame=?, "
                "frame_size=?, args_est=?, ret_pop=?, indirect_calls=?, indirect_jumps=?, invalid=?, style=?, "
                "regs_in=?, flags_in=?, fpu_in=?, difficulty=?, norm_hash=?, reg_bases=? WHERE addr=?",
                (r["end"], r["size"], r["n_insn"], r["n_cond"], r["x87"], r["mmx"], r["sse"], int(r["has_frame"]),
                 r["frame_size"], r["args_est"], r["ret_pop"], r["indirect_calls"], r["indirect_jumps"],
                 int(r["invalid"]), r["style"], ",".join(r["regs_in"]), int(r["flags_in"]), int(r["fpu_in"]),
                 difficulty(r), r["norm_hash"], json.dumps(r["reg_bases"]), addr))
            con.execute("DELETE FROM edges WHERE src=?", (addr,))
            con.execute("DELETE FROM func_imports WHERE addr=?", (addr,))
            con.execute("DELETE FROM func_globals WHERE addr=?", (addr,))
            con.execute("DELETE FROM jtables WHERE addr=?", (addr,))
            con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)",
                            [(addr, d, "call") for d in r["calls"]] + [(addr, d, "tail") for d in r["tails"]])
            con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(addr, i) for i in r["imports"]])
            con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(addr, g) for g in r["globals"]])
            con.executemany("INSERT INTO jtables VALUES(?,?,?)", [(addr, t, n) for t, n in r["jtables"]])
            con.executemany("INSERT OR REPLACE INTO strings VALUES(?,?)",
                            [(int(g), s) for g, s in r["strings"].items()])
            con.execute("INSERT INTO events(ts, addr, agent, event, detail) VALUES(?,?,?,?,?)",
                        (now, addr, "lead", "reextent", json.dumps({"old_size": old, "new_size": new,
                                                                     "past": [hex(d) for d in cov[:8]]})))
    with open(ROOT / "build" / "reextent.txt", "a") as fh:
        for addr, state, old, new, cov, _ in changed:
            fh.write(f"{addr:#010x} {state} size {old} -> {new} past {' '.join(hex(d) for d in cov)}\n")
    print(f"updated {len(changed)} functions")


if __name__ == "__main__":
    main()
