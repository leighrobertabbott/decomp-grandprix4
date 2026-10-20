"""Lead helper: repair the analysis around `add reg,TABLE; jmp|call [reg]` dispatches.

    python tools/fix_switches.py [--lo 0x510000 --hi 0x560000] [--dry]

The analysis only recognises `jmp [reg*4+table]` switches. For the R3 form
`add reg,TABLE; jmp [reg]` (and `call [reg]` handler tables) it leaves
  - the TABLE dwords queued as a "function" (data decoded as code),
  - for `jmp`: the case bodies outside their parent (the shape gate then reports their
    callees/globals as extra) and sometimes queued as functions of their own (tails),
  - for `call`: handler starts that were never registered.
For every such site this tool bounds the table (entries must be executable code within
0x4000 bytes; the table ends before the lowest target that follows it), then
  - excludes queued rows that start inside the table (`not a function: jump table`),
  - jmp: merges the case bodies' calls/imports/globals into the parent (as
    tools/merge_cases.py does) and excludes queued rows at case targets that nothing else
    calls (`not a function: case body`),
  - call: registers missing handler starts (as tools/add_starts.py does).
Accepted or claimed rows are never touched (they are listed). The verifier still decides.
"""
import argparse
import json
import re
import sqlite3
import struct
import sys
import time
from pathlib import Path

import capstone

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from gp4re.analyze import Analyzer           # noqa: E402
from gp4re.db import difficulty, track_for   # noqa: E402
from gp4re.verify import gp4_binary          # noqa: E402

SITE = re.compile(r"(call|jmp) dword ptr (?:cs:)?\[(e[a-d]x|e[sd]i|ebp)\]$")
ADD = re.compile(r"add (\w+), (0x[0-9a-f]+)$")


def is_code(b, va):
    sec = b.section_of(va)
    return bool(sec and sec.characteristics & 0x20000000)


def table_targets(b, table: int) -> list[int]:
    """Entries of an add-table dispatch: executable targets within 0x4000 bytes, ending before
    the lowest target that follows the table."""
    ents = []
    for k in range(256):
        v = struct.unpack("<I", b.read(table + 4 * k, 4))[0]
        if not is_code(b, v) or abs(v - table) > 0x4000:
            break
        ents.append(v)
        after = [t for t in ents if t > table]
        if after and table + 4 * (k + 1) >= min(after):
            break
    after = [t for t in ents if t > table]
    if after:
        ents = ents[:(min(after) - table) // 4]
    return ents


def closure(b, con, lo: int, hi: int, dry: bool) -> None:
    """Explore every live function and follow its add-table `jmp [reg]` switches recursively
    (case bodies that hold switches of their own included); merge what the case bodies call,
    import and read into the function. Attributes switches inside shared case bodies to the
    functions that really reach them, which the per-row pass cannot."""
    live = {r[0] for r in con.execute(
        "SELECT f.addr FROM functions f JOIN ledger l ON l.addr=f.addr "
        "WHERE f.kind!='data' AND l.state!='excluded'")}
    an = Analyzer(b, log=lambda *x: None)
    # pass 1: which live functions reach each case body; a body reached by two or more is a
    # shared tail and becomes a function of its own (like any jump target shared by functions).
    # Rounds register only the shallowest shared bodies (fewest table hops from a function), so
    # code reached only through one shared body stays part of it.
    shared_all: list[int] = []
    while True:
        reach: dict[int, set[int]] = {}
        depth: dict[int, int] = {}
        for p in sorted(x for x in live if lo <= x < hi):
            an.starts = live - {p}
            queue, seen = [(p, 0)], set()
            while queue:
                t, d = queue.pop(0)
                if t in seen:
                    continue
                seen.add(t)
                f = an.explore(t)
                ins = [f.insns[v] for v in sorted(f.insns)]
                for prev, cur in zip(ins, ins[1:]):
                    m = SITE.match(f"{cur.mn} {cur.ops}")
                    pm = ADD.match(f"{prev.mn} {prev.ops}")
                    if m and m.group(1) == "jmp" and pm and pm.group(1) == m.group(2) and prev.va + prev.size == cur.va:
                        for v in table_targets(b, int(pm.group(2), 16)):
                            if v not in live:
                                reach.setdefault(v, set()).add(p)
                                depth[v] = min(depth.get(v, 1 << 30), d + 1)
                                queue.append((v, d + 1))
        cands = [v for v, ps in reach.items() if len(ps) >= 2]
        if not cands:
            break
        dmin = min(depth[v] for v in cands)
        new_shared = sorted(v for v in cands if depth[v] == dmin)
        shared_all += new_shared
        live |= set(new_shared)
    shared = sorted(shared_all)
    print(f"shared case bodies: {len(shared)} {[hex(v) for v in shared][:60]}")
    have = {r[0] for r in con.execute("SELECT addr FROM functions")}
    revive = [v for v in shared if v in have]      # rows excluded earlier as case bodies
    fresh = [v for v in shared if v not in have]
    print(f"  re-include {[hex(v) for v in revive]}; add {len(fresh)}")
    if shared and not dry:
        import subprocess
        with con:
            for v in revive:
                con.execute("UPDATE ledger SET state='todo', why=NULL, owner=NULL, updated=? WHERE addr=? "
                            "AND state='excluded'", (time.time(), v))
                con.execute("UPDATE functions SET kind='game', kind_evidence='lead: switch case body shared by "
                            "several functions (fix_switches --closure)' WHERE addr=?", (v,))
        if fresh:
            subprocess.run([sys.executable, str(ROOT / "tools" / "add_starts.py"), "--why",
                            "switch case body shared by several functions (fix_switches --closure)",
                            *[hex(v) for v in fresh]], check=True)
    added = 0
    rows = []
    for p in sorted(x for x in live if lo <= x < hi):
        an.starts = live - {p}
        queue, seen, switched = [p], set(), False
        calls, imports, globals_ = set(), set(), set()
        while queue:
            t = queue.pop()
            if t in seen:
                continue
            seen.add(t)
            f = an.explore(t)
            calls |= f.calls | f.tails
            imports |= f.imports
            globals_ |= f.globals_r
            ins = [f.insns[v] for v in sorted(f.insns)]
            for prev, cur in zip(ins, ins[1:]):
                m = SITE.match(f"{cur.mn} {cur.ops}")
                pm = ADD.match(f"{prev.mn} {prev.ops}")
                if m and m.group(1) == "jmp" and pm and pm.group(1) == m.group(2) and prev.va + prev.size == cur.va:
                    table = int(pm.group(2), 16)
                    globals_.discard(table)
                    for v in table_targets(b, table):
                        switched = True
                        if v in live:
                            calls.add(v)        # a case that is a function of its own: a tail
                        else:
                            queue.append(v)
        if not switched:
            continue
        calls.discard(p)
        have_c = {r[0] for r in con.execute("SELECT dst FROM edges WHERE src=?", (p,))}
        have_i = {r[0] for r in con.execute("SELECT import FROM func_imports WHERE addr=?", (p,))}
        have_g = {r[0] for r in con.execute("SELECT gaddr FROM func_globals WHERE addr=?", (p,))}
        new = (calls - have_c, imports - have_i, globals_ - have_g)
        if any(new):
            rows.append((p, *new))
            added += sum(len(x) for x in new)
            print(f"  {p:#010x} +calls {[hex(x) for x in sorted(new[0])]} +imports {sorted(new[1])} +globals {len(new[2])}")
    print(f"closure: {len(rows)} functions gain {added} rows")
    if dry:
        return
    with con:
        for p, c, i, g in rows:
            con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)", [(p, x, "call") for x in c])
            con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(p, x) for x in i])
            con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(p, x) for x in g])
    print("applied")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--lo", default="0x510000")
    ap.add_argument("--hi", default="0x560000")
    ap.add_argument("--dry", action="store_true")
    ap.add_argument("--closure", action="store_true", help="merge add-table switch case bodies by reachability")
    a = ap.parse_args()
    lo, hi = int(a.lo, 16), int(a.hi, 16)
    b = gp4_binary()
    con = sqlite3.connect(ROOT / "state" / "gp4re.sqlite")
    if a.closure:
        closure(b, con, lo, hi, a.dry)
        return
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    funcs = {r[0]: (r[1], r[2]) for r in con.execute("SELECT addr, end_addr, kind FROM functions")}
    state = {r[0]: r[1] for r in con.execute("SELECT addr, state FROM ledger")}
    callers: dict[int, set[int]] = {}
    for s, d in con.execute("SELECT src, dst FROM edges WHERE kind='call'"):
        callers.setdefault(d, set()).add(s)
    regions = con.execute("SELECT start, end_addr, label FROM regions").fetchall()

    sites = []
    for p, (e, k) in sorted(funcs.items()):
        if k == "data" or not (lo <= p < hi):
            continue
        prev = None
        for ins in md.disasm(b.read(p, min(e - p, 0x2000)), p):
            m = SITE.match(f"{ins.mnemonic} {ins.op_str}")
            if m and prev is not None:
                pm = ADD.match(f"{prev.mnemonic} {prev.op_str}")
                if pm and pm.group(1) == m.group(2):
                    sites.append((p, ins.address, m.group(1), int(pm.group(2), 16)))
            prev = ins

    excl, merges, adds, kept = [], {}, set(), []
    done_tables = set()
    for p, site, kind, table in sites:
        if (table, kind) in done_tables:
            continue
        done_tables.add((table, kind))
        ents = []
        for k in range(256):
            v = struct.unpack("<I", b.read(table + 4 * k, 4))[0]
            if not is_code(b, v) or abs(v - table) > 0x4000:
                break
            ents.append(v)
            after = [t for t in ents if t > table]
            if after and table + 4 * (k + 1) >= min(after):
                break
        if not ents:
            continue
        after = [t for t in ents if t > table]
        n = len(ents)
        if after:
            n = min(n, (min(after) - table) // 4)
        ents = ents[:n]
        tend = table + 4 * n
        targets = sorted(set(ents))
        for r, (_, k) in funcs.items():
            if table <= r < tend and k != "data":
                if state.get(r) in ("todo", "deferred"):
                    excl.append((r, f"not a function: jump table {table:#x} ({n} dwords) of {p:#x} "
                                    f"(add reg,{table:#x}; {kind} [reg] at {site:#x})"))
                else:
                    kept.append((r, state.get(r), f"inside table {table:#x}"))
        if kind == "jmp":
            merges.setdefault(p, set()).update(targets)
            for t in targets:
                if t in funcs and funcs[t][1] != "data":
                    others = callers.get(t, set()) - {p}
                    if others:
                        kept.append((t, state.get(t), f"case target of {p:#x} also called by "
                                                      f"{sorted(hex(x) for x in others)[:3]}"))
                    elif state.get(t) in ("todo", "deferred"):
                        excl.append((t, f"not a function: case body of the switch at {site:#x} in {p:#x} "
                                        f"(table {table:#x})"))
                    else:
                        kept.append((t, state.get(t), f"case body of {p:#x}"))
        else:
            for t in targets:
                if t not in funcs:
                    adds.add(t)
    excl_addrs = {x for x, _ in excl}
    adds -= excl_addrs
    # a case body that holds a switch of its own: its cases belong to the outer parent too
    changed = True
    while changed:
        changed = False
        for p, targets in merges.items():
            for t in list(targets):
                if t != p and t in merges and t in excl_addrs and not merges[t] <= targets:
                    targets |= merges[t]
                    changed = True

    an = Analyzer(b, log=lambda *x: None)
    base_starts = {r for r, (_, k) in funcs.items() if k != "data"} - excl_addrs
    merge_rows = {}
    for p, targets in merges.items():
        an.starts = base_starts - set(targets)
        calls, imports, globals_ = set(), set(), set()
        for t in targets:
            f = an.explore(t)
            calls |= f.calls | f.tails
            imports |= f.imports
            globals_ |= f.globals_r
        calls -= excl_addrs | {p}   # a case body jumping back to its parent's start is a loop
        merge_rows[p] = (calls, imports, globals_)
    an.starts = base_starts | adds
    add_rows = []
    for s in sorted(adds):
        f = an.explore(s, "lead")
        if s not in f.insns:
            kept.append((s, None, "handler target does not decode"))
            continue
        r = an.summarize(f)
        reg = next((lab for rlo, rhi, lab in regions if rlo <= s < rhi), None)
        add_rows.append((r, reg))

    print(f"sites {len(sites)} tables {len(done_tables)}")
    print(f"exclude {len(excl)}:")
    for x, why in sorted(excl):
        print(f"  {x:#010x} {why}")
    print(f"merge into {len(merge_rows)} parents:")
    for p, (c, i, g) in sorted(merge_rows.items()):
        print(f"  {p:#010x} calls {[hex(x) for x in sorted(c)]} imports {sorted(i)} globals {len(g)}")
    print(f"add {len(add_rows)} handler starts: {[hex(r['addr']) for r, _ in add_rows]}")
    print(f"left alone {len(kept)}:")
    for x, s, why in sorted(kept, key=lambda t: t[0]):
        print(f"  {x:#010x} [{s}] {why}")
    if a.dry:
        return
    now = time.time()
    with con:
        for x, why in excl:
            con.execute("UPDATE ledger SET state='excluded', why=?, owner=NULL, updated=? WHERE addr=? "
                        "AND state IN ('todo','deferred')", (why, now, x))
            con.execute("UPDATE functions SET kind='data' WHERE addr=?", (x,))
            con.execute("INSERT INTO events(ts, addr, agent, event, detail) VALUES(?,?,?,?,?)",
                        (now, x, "lead", "exclude", json.dumps({"why": why, "tool": "fix_switches"})))
        con.execute("DELETE FROM edges WHERE kind='tail' AND dst IN (SELECT addr FROM functions WHERE kind='data')")
        for p, (c, i, g) in merge_rows.items():
            con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)", [(p, x, "call") for x in c])
            con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(p, x) for x in i])
            con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(p, x) for x in g])
        for r, reg in add_rows:
            con.execute(
                "INSERT INTO functions VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                (r["addr"], r["end"], r["size"], r["n_insn"], r["n_cond"], r["x87"], r["mmx"], r["sse"],
                 int(r["has_frame"]), r["frame_size"], r["args_est"], r["ret_pop"], r["indirect_calls"],
                 r["indirect_jumps"], int(r["invalid"]), r["source"], r["style"], ",".join(r["regs_in"]),
                 int(r["flags_in"]), int(r["fpu_in"]), difficulty(r), "game",
                 "lead: handler start of an add-table call dispatch (fix_switches)",
                 reg, r["norm_hash"], json.dumps(r["reg_bases"]), ""))
            con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)",
                            [(r["addr"], d, "call") for d in r["calls"]] +
                            [(r["addr"], d, "tail") for d in r["tails"]])
            con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(r["addr"], x) for x in r["imports"]])
            con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(r["addr"], x) for x in r["globals"]])
            con.execute("INSERT OR IGNORE INTO ledger(addr, state, track, attempts, priority, why, updated) "
                        "VALUES(?,?,?,0,0,NULL,?)", (r["addr"], "todo", track_for(r), now))
    with open(ROOT / "build" / "missing_starts.txt", "a") as fh:
        for r, _ in add_rows:
            fh.write(f"{r['addr']:#010x} added (fix_switches: add-table call dispatch handler)\n")
    print("applied")


if __name__ == "__main__":
    main()
