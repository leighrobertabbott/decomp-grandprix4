"""Lead helper: register function starts the analysis missed (handlers reached only through
an unrecognised dispatch table, `call [reg]` targets, ...).

    python tools/add_starts.py --why "<evidence>" ADDR [ADDR ...] [--dry]

Each start is explored with the analyzer (stopping at known function starts and at the other
new starts), summarised exactly like `gp4re analyze` does, and inserted into `functions`,
`edges`, `func_imports`, `func_globals`, `jtables` and the ledger (state todo). Starts that
already exist, that are not executable code, or that are not instruction boundaries of their
own exploration are refused. Rerunning `gp4re analyze` rebuilds the analysis tables and drops
these rows (the ledger rows stay); record the starts in `build/missing_starts.txt` as well.
"""
import argparse
import json
import sqlite3
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from gp4re.analyze import Analyzer           # noqa: E402
from gp4re.db import difficulty, track_for   # noqa: E402
from gp4re.verify import gp4_binary          # noqa: E402


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("addrs", nargs="+")
    ap.add_argument("--why", required=True)
    ap.add_argument("--dry", action="store_true")
    a = ap.parse_args()
    new = [int(x, 16) for x in a.addrs]
    b = gp4_binary()
    con = sqlite3.connect(ROOT / "state" / "gp4re.sqlite")
    known = {r[0] for r in con.execute("SELECT addr FROM functions")}
    for s in new:
        if s in known:
            sys.exit(f"{s:#x} is already a function row")
        sec = b.section_of(s)
        if not sec or not (sec.characteristics & 0x20000000):
            sys.exit(f"{s:#x} is not in executable code")
    an = Analyzer(b, log=lambda *x: None)
    an.starts = {r[0] for r in con.execute("SELECT addr FROM functions WHERE kind!='data'")} | set(new)
    regions = con.execute("SELECT start, end_addr, label FROM regions").fetchall()
    rows = []
    for s in new:
        f = an.explore(s, "lead")
        if s not in f.insns:
            sys.exit(f"{s:#x} does not decode as an instruction")
        r = an.summarize(f)
        reg = next((lab for lo, hi, lab in regions if lo <= s < hi), None)
        rows.append((r, reg))
        print(f"{s:#010x} size={r['size']} insns={r['n_insn']} style={r['style']} regs_in={r['regs_in']} "
              f"calls={[hex(c) for c in r['calls']]} tails={[hex(c) for c in r['tails']]} region={reg}")
    if a.dry:
        return
    now = time.time()
    with con:
        for r, reg in rows:
            con.execute(
                "INSERT INTO functions VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                (r["addr"], r["end"], r["size"], r["n_insn"], r["n_cond"], r["x87"], r["mmx"], r["sse"],
                 int(r["has_frame"]), r["frame_size"], r["args_est"], r["ret_pop"], r["indirect_calls"],
                 r["indirect_jumps"], int(r["invalid"]), r["source"], r["style"], ",".join(r["regs_in"]),
                 int(r["flags_in"]), int(r["fpu_in"]), difficulty(r), "game", f"lead: missing start ({a.why})",
                 reg, r["norm_hash"], json.dumps(r["reg_bases"]), ""))
            con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)",
                            [(r["addr"], d, "call") for d in r["calls"]] +
                            [(r["addr"], d, "tail") for d in r["tails"]])
            con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(r["addr"], i) for i in r["imports"]])
            con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(r["addr"], g) for g in r["globals"]])
            con.executemany("INSERT INTO jtables VALUES(?,?,?)", [(r["addr"], t, n) for t, n in r["jtables"]])
            con.execute("INSERT OR IGNORE INTO ledger(addr, state, track, attempts, priority, why, updated) "
                        "VALUES(?,?,?,0,0,NULL,?)", (r["addr"], "todo", track_for(r), now))
            con.execute("INSERT INTO events(ts, addr, agent, event, detail) VALUES(?,?,?,?,?)",
                        (now, r["addr"], "lead", "add_start", json.dumps({"why": a.why})))
    with open(ROOT / "build" / "missing_starts.txt", "a") as fh:
        for r, _ in rows:
            fh.write(f"{r['addr']:#010x} added ({a.why})\n")
    print(f"added {len(rows)} starts")


if __name__ == "__main__":
    main()
