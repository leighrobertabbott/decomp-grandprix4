"""gp4re command line. Run from the repository root:  python -m gp4re <command> ...

Workers prefix every call with their id:  GP4RE_AGENT=w01 python -m gp4re work claim ...
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import time
from pathlib import Path

from . import db as dbm
from . import work
from .paths import REPO, analysis_dir, find_exe, main_root
from .work import hx, parse_addr


def _p(obj, as_json=False):
    print(json.dumps(obj, indent=1, default=str) if as_json or not isinstance(obj, str) else obj)


# ------------------------------------------------------------------ analyze
def cmd_analyze(a):
    from .analyze import analyze
    from .binary import Binary
    from .classify import classify
    exe = Path(a.exe) if a.exe else find_exe()
    b = Binary.load(exe)
    fp = b.fingerprint()
    (analysis_dir() / "binary.json").write_text(json.dumps(fp, indent=2))
    for w in fp["warnings"]:
        print("warning:", w)
    if any("packed/encrypted" in w for w in fp["warnings"]):
        raise SystemExit("refusing to analyse: the code section is not readable as plain code")
    print(f"{fp['file']} sha256={fp['sha256'][:16]}... base={fp['image_base']} entry={fp['entry']}")
    from .config import cfg
    pinned = cfg()["game"]["sha256"]
    if pinned and pinned != fp["sha256"] and not a.allow_other_exe:
        raise SystemExit(f"this GP4.exe (sha256 {fp['sha256']}) is not the pinned build {pinned} - every address "
                         f"in the project assumes the pinned build. Use --allow-other-exe to analyse it anyway.")
    rows, stats = analyze(b, log=print)
    kinds = classify(rows, b.entry)
    from .classify import tags_for
    for r in rows:
        r["tags"] = tags_for(r)
    regions = []
    rf = REPO / "progress" / "regions.json"
    if rf.exists():
        rj = json.loads(rf.read_text())
        if rj.get("exe_sha256") == fp["sha256"]:
            for g in rj["regions"]:
                regions.append({**g, "start": int(g["start"], 16), "end": int(g["end"], 16)})
        else:
            print("note: progress/regions.json is for a different GP4.exe; regions not applied")
    con = dbm.connect()
    res = dbm.load_analysis(con, rows, stats, fp, kinds, regions)
    with dbm.tx(con):
        if a.fpucw:
            dbm.meta_set(con, "fpucw", int(a.fpucw, 16))
    with open(analysis_dir() / "functions.jsonl", "w") as fh:
        for r in rows:
            fh.write(json.dumps({**r, "strings": {hex(int(k)): v for k, v in r["strings"].items()}}) + "\n")
    import collections
    print(f"functions: {len(rows)}  kinds: {dict(collections.Counter(r['kind'] for r in rows))}")
    print(f"styles: {dict(collections.Counter(r['style'] for r in rows))}")
    print(f"ledger: +{res['new_ledger_rows']} new, {res['orphaned']} orphaned  ->  {dbm.connect and 'state/gp4re.sqlite'}")


def cmd_status(a):
    con = dbm.connect()
    s = work.status(con)
    if a.json:
        return _p(s, True)
    print(f"GP4 progress: {s['progress_pct']}% of game code bytes accepted "
          f"({s['accepted_bytes']}/{s['game_bytes']})   active goal: {s['active_goal']}   x87 CW: {s['fpucw']}")
    print("ledger:", ", ".join(f"{k}={v}" for k, v in sorted(s["by_state"].items())))
    for t, c in s["by_track"].items():
        print(f"  track {t}: " + ", ".join(f"{k}={v}" for k, v in sorted(c.items())))
    print(f"names: {s['names']}  open name conflicts: {s['name_conflicts']}")


def cmd_doctor(a):
    ok = True

    def chk(label, fn):
        nonlocal ok
        try:
            msg = fn()
            print(f"  ok    {label}: {msg}")
        except Exception as e:  # noqa: BLE001
            ok = False
            print(f"  FAIL  {label}: {e}")
    import importlib
    for m in ("pefile", "capstone", "unicorn"):
        chk(f"python module {m}", lambda m=m: importlib.import_module(m).__name__)
    chk("GP4.exe", lambda: str(find_exe()))
    from .msvc import msvc_env
    chk("MSVC x86 toolchain", lambda: "cl/link via vcvars32 " + ("ok" if msvc_env().get("INCLUDE") else "?"))
    con = dbm.connect()
    chk("analysis", lambda: f"{con.execute('SELECT COUNT(*) FROM functions').fetchone()[0]} functions"
        if con.execute("SELECT COUNT(*) FROM functions").fetchone()[0] else (_ for _ in ()).throw(
            RuntimeError("run: python -m gp4re analyze")))
    import os
    import shutil
    try:
        from .ghidra import ghidra_dir
        gh = ghidra_dir()
        n = con.execute("SELECT COUNT(*) FROM pseudocode").fetchone()[0]
        print(f"  ok    Ghidra: {gh.name}; pseudocode cached for {n} functions")
    except SystemExit:
        print("  info  Ghidra (optional, adds pseudocode to packets): not installed")
    print(f"  info  main checkout: {main_root()}")
    print(f"  info  claude CLI on PATH (only needed for unattended runs): {shutil.which('claude') or 'not on PATH'}")
    sys.exit(0 if ok else 1)


# ------------------------------------------------------------------ work
def cmd_claim(a):
    con = dbm.connect()
    agent = work.agent_id(a)
    got = work.claim(con, agent, a.count, a.min_size, a.max_size, a.track, a.region, a.goal,
                     parse_addr(a.addr) if a.addr else None, a.tag)
    if not got:
        print(json.dumps({"agent": agent, "empty": True}))
        return
    rows = [con.execute("SELECT f.addr, f.size, f.style, l.track FROM functions f JOIN ledger l ON l.addr=f.addr "
                        "WHERE f.addr=?", (x,)).fetchone() for x in got]
    print(json.dumps({"agent": agent, "empty": False,
                      "claimed": [{"addr": hx(r["addr"]), "name": dbm.name_of(con, r["addr"]), "size": r["size"],
                                   "track": r["track"], "style": r["style"],
                                   "scratch": f"build/scratch/{r['addr']:08x}/"} for r in rows]}))
    if a.context:
        for x in got:
            print()
            print(work.packet(con, x))


def cmd_show(a):
    print(work.packet(dbm.connect(), parse_addr(a.addr), a.asm))


def cmd_try(a):
    con = dbm.connect()
    res = work.try_candidate(con, parse_addr(a.addr), a.file, work.agent_id(a), a.cap)
    _p(res, True) if a.json else print(work.render_verdict(res))
    # non-zero unless it passed, so "try ... && accept ..." only accepts a pass
    sys.exit(0 if res.get("overall") in ("pass", "pass-pending-runtime") else 1)


def cmd_accept(a):
    con = dbm.connect()
    res = work.accept(con, parse_addr(a.addr), a.file, work.agent_id(a))
    if a.json:
        return _p(res, True)
    if res["accepted"]:
        print(f"ACCEPTED {res['addr']} -> {res['dest']} ({res['state']})")
    else:
        print(work.render_verdict(res))
        print("NOT ACCEPTED")
        sys.exit(1)


def cmd_defer(a):
    con = dbm.connect()
    work.defer(con, parse_addr(a.addr), work.agent_id(a), a.why, a.needs)
    print(f"DEFERRED {hx(parse_addr(a.addr))}")


def cmd_release(a):
    work.release(dbm.connect(), parse_addr(a.addr), work.agent_id(a), a.note)
    print(f"RELEASED {hx(parse_addr(a.addr))}")


def cmd_name(a):
    con = dbm.connect()
    r = work.name(con, parse_addr(a.addr), a.name, a.type, work.agent_id(a), a.evidence, a.kind, a.force)
    print(f"{hx(parse_addr(a.addr))} {a.name}: {r}")


def _ranges(spec: str):
    if "-" in spec:
        lo, hi = spec.split("-")
        return parse_addr(lo), parse_addr(hi)
    x = parse_addr(spec)
    return x, x + 1


def cmd_exclude(a, include=False):
    con = dbm.connect()
    lo, hi = _ranges(a.target)
    with dbm.tx(con):
        if include:
            n = con.execute("UPDATE ledger SET state='todo', why=NULL, updated=? WHERE addr>=? AND addr<? "
                            "AND state='excluded'", (time.time(), lo, hi)).rowcount
            con.execute("UPDATE functions SET kind='game', kind_evidence='included by lead' "
                        "WHERE addr>=? AND addr<?", (lo, hi))
        else:
            n = con.execute("UPDATE ledger SET state='excluded', why=?, owner=NULL, updated=? WHERE addr>=? "
                            "AND addr<? AND state IN ('todo','deferred','claimed')",
                            (a.why or "excluded by lead", time.time(), lo, hi)).rowcount
            con.execute("UPDATE functions SET kind=COALESCE(?, kind) WHERE addr>=? AND addr<?", (a.kind, lo, hi))
        dbm.log_event(con, lo, work.agent_id(a), "include" if include else "exclude", {"hi": hi, "n": n, "why": a.why})
    print(f"{'included' if include else 'excluded'} {n} functions in {hx(lo)}-{hx(hi)}")


def cmd_requeue(a):
    con = dbm.connect()
    with dbm.tx(con):
        n = 0
        for s in a.addrs:
            n += con.execute("UPDATE ledger SET state='todo', owner=NULL, priority=?, attempts=0, updated=? "
                             "WHERE addr=? AND state IN ('deferred','claimed','orphaned')",
                             (a.priority, time.time(), parse_addr(s))).rowcount
    print(f"requeued {n}")


def cmd_sweep(a):
    _p(work.sweep(dbm.connect(), a.hours), True)


def cmd_names(a):
    con = dbm.connect()
    if a.action == "conflicts":
        for r in con.execute("SELECT p.*, n.name AS cur FROM names_proposed p LEFT JOIN names n ON n.addr=p.addr "
                             "WHERE p.resolved=0 ORDER BY p.addr"):
            print(f"{hx(r['addr'])} current={r['cur']} proposed={r['name']} by {r['agent']}: {r['evidence']}")
    elif a.action == "list":
        for r in con.execute("SELECT * FROM names ORDER BY addr"):
            print(f"{hx(r['addr'])} {r['name']} {r['kind']} {r['conf']} {r['type'] or ''}")


def cmd_goal(a):
    con = dbm.connect()
    if a.action == "list":
        for r in con.execute("SELECT goal, COUNT(*) n FROM goals GROUP BY goal"):
            print(f"{r['goal']}: {r['n']} functions")
        print("active:", dbm.meta_get(con, "active_goal"))
    elif a.action == "set":
        with dbm.tx(con):
            dbm.meta_set(con, "active_goal", a.name)
        print("active goal:", a.name)
    elif a.action == "clear":
        with dbm.tx(con):
            dbm.meta_set(con, "active_goal", None)
        print("active goal cleared")
    elif a.action == "closure":
        print(f"goal {a.name}: {work.goal_closure(con, a.name, parse_addr(a.arg), a.depth)} functions")
    elif a.action == "import":
        print(f"goal {a.name}: +{work.goal_import(con, a.name, Path(a.arg))} functions from {a.arg}")
    elif a.action == "region":
        with dbm.tx(con):
            n = con.execute("INSERT OR IGNORE INTO goals SELECT ?, addr FROM functions WHERE region LIKE ? "
                            "AND kind='game'", (a.name, a.arg + "%")).rowcount
        print(f"goal {a.name}: +{n} functions from region {a.arg}")


def cmd_asm(a):
    from .verify import gp4_binary
    print("\n".join(work.listing(dbm.connect(), gp4_binary(), parse_addr(a.addr))))


def cmd_xref(a):
    con = dbm.connect()
    x = parse_addr(a.addr)
    for label, sql in (("called by", "SELECT src AS a FROM edges WHERE dst=?"),
                       ("calls", "SELECT dst AS a FROM edges WHERE src=?"),
                       ("referenced by (data)", "SELECT addr AS a FROM func_globals WHERE gaddr=?")):
        rows = [r["a"] for r in con.execute(sql, (x,))]
        if rows:
            print(f"{label} ({len(rows)}): " + ", ".join(f"{hx(r)} {dbm.name_of(con, r)}" for r in rows[:60]))


def cmd_find(a):
    con = dbm.connect()
    pat = re.compile(a.pattern, re.I)
    for r in con.execute("SELECT s.gaddr, s.text FROM strings s"):
        if pat.search(r["text"]):
            users = [u[0] for u in con.execute("SELECT addr FROM func_globals WHERE gaddr=?", (r["gaddr"],))]
            print(f'{hx(r["gaddr"])} "{r["text"][:70]}" <- ' + ", ".join(hx(u) for u in users[:6]))
    for r in con.execute("SELECT addr, name FROM names"):
        if pat.search(r["name"]):
            print(f"{hx(r['addr'])} name {r['name']}")


def cmd_verify(a):
    from . import verify
    res = verify.run(parse_addr(a.addr), candidate=Path(a.file) if a.file else None, agent=work.agent_id(a),
                     cases=a.cases)
    res["why"] = work._why(res)
    _p(res, True) if a.json else print(work.render_verdict(res))


def cmd_regress(a):
    """Re-run every gate on every accepted function; report any that no longer pass."""
    from . import verify
    con = dbm.connect()
    rows = con.execute("SELECT addr, state FROM ledger WHERE state IN ('accepted','accepted-runtime') "
                       "ORDER BY addr").fetchall()
    broken = []
    for r in rows:
        res = verify.run(r["addr"], agent="regress", cases=a.cases)
        ok = res["overall"] in ("pass", "pass-pending-runtime")
        print(f"{hx(r['addr'])} {dbm.name_of(con, r['addr'])}: {res['overall']}")
        if not ok:
            broken.append(r["addr"])
            print("   " + work._why(res)[:400])
    print(f"regression: {len(rows) - len(broken)}/{len(rows)} accepted functions still pass")
    sys.exit(1 if broken else 0)


def cmd_export(a):
    print("wrote", ", ".join(work.export(dbm.connect())))


def cmd_fpucw(a):
    con = dbm.connect()
    with dbm.tx(con):
        dbm.meta_set(con, "fpucw", int(a.value, 16))
    print("x87 control word for the emulator oracle:", a.value)


def cmd_ghidra(a):
    from . import ghidra
    ghidra.main(a)


def cmd_runtime(a):
    from . import runtime
    runtime.main(a)


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="gp4re", description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--agent", help="agent id (default: $GP4RE_AGENT or 'lead')")
    sub = p.add_subparsers(dest="cmd", required=True)

    s = sub.add_parser("analyze", help="(lead) fingerprint + discover functions + classify + load the ledger")
    s.add_argument("--exe"); s.add_argument("--fpucw", help="x87 control word for the oracle, e.g. 007f")
    s.add_argument("--allow-other-exe", action="store_true", help="analyse a build other than the pinned one")
    s.set_defaults(fn=cmd_analyze)
    s = sub.add_parser("status"); s.add_argument("--json", action="store_true"); s.set_defaults(fn=cmd_status)
    sub.add_parser("doctor").set_defaults(fn=cmd_doctor)

    w = sub.add_parser("work", help="the per-function loop").add_subparsers(dest="wcmd", required=True)
    s = w.add_parser("claim")
    s.add_argument("--count", type=int, default=1); s.add_argument("--min-size", type=int)
    s.add_argument("--max-size", type=int); s.add_argument("--track", choices=["asm", "c"])
    s.add_argument("--region"); s.add_argument("--goal"); s.add_argument("--addr")
    s.add_argument("--tag", help="only functions tagged e.g. physics, car, ai, track")
    s.add_argument("--context", action="store_true", help="print each claimed function's context packet")
    s.set_defaults(fn=cmd_claim)
    s = w.add_parser("show"); s.add_argument("addr"); s.add_argument("--asm", action="store_true"); s.set_defaults(fn=cmd_show)
    s = w.add_parser("try"); s.add_argument("addr"); s.add_argument("file")
    s.add_argument("--cap", type=int, default=work.DEFAULT_CAP); s.add_argument("--json", action="store_true")
    s.set_defaults(fn=cmd_try)
    s = w.add_parser("accept"); s.add_argument("addr"); s.add_argument("file"); s.add_argument("--json", action="store_true")
    s.set_defaults(fn=cmd_accept)
    s = w.add_parser("defer"); s.add_argument("addr"); s.add_argument("why"); s.add_argument("--needs")
    s.set_defaults(fn=cmd_defer)
    s = w.add_parser("release"); s.add_argument("addr"); s.add_argument("--note"); s.set_defaults(fn=cmd_release)
    s = w.add_parser("name"); s.add_argument("addr"); s.add_argument("name"); s.add_argument("--type")
    s.add_argument("--evidence", required=True); s.add_argument("--kind", default="func", choices=["func", "global", "type"])
    s.add_argument("--force", action="store_true", help="(lead) set as confirmed, resolving conflicts")
    s.set_defaults(fn=cmd_name)
    s = w.add_parser("exclude", help="(lead) addr or start-end"); s.add_argument("target"); s.add_argument("--why")
    s.add_argument("--kind"); s.set_defaults(fn=cmd_exclude)
    s = w.add_parser("include", help="(lead) addr or start-end"); s.add_argument("target")
    s.set_defaults(fn=lambda a: cmd_exclude(a, include=True))
    s = w.add_parser("requeue", help="(lead) deferred -> todo"); s.add_argument("addrs", nargs="+")
    s.add_argument("--priority", type=int, default=1); s.set_defaults(fn=cmd_requeue)
    s = w.add_parser("sweep", help="(lead) batch report"); s.add_argument("--hours", type=float, default=6)
    s.set_defaults(fn=cmd_sweep)

    s = sub.add_parser("names", help="(lead) naming pass"); s.add_argument("action", choices=["conflicts", "list"])
    s.set_defaults(fn=cmd_names)
    s = sub.add_parser("goal", help="(lead) scope the queue")
    s.add_argument("action", choices=["list", "set", "clear", "closure", "import", "region"])
    s.add_argument("name", nargs="?"); s.add_argument("arg", nargs="?"); s.add_argument("--depth", type=int, default=6)
    s.set_defaults(fn=cmd_goal)
    s = sub.add_parser("asm"); s.add_argument("addr"); s.set_defaults(fn=cmd_asm)
    s = sub.add_parser("xref"); s.add_argument("addr"); s.set_defaults(fn=cmd_xref)
    s = sub.add_parser("find", help="search strings and names"); s.add_argument("pattern"); s.set_defaults(fn=cmd_find)
    s = sub.add_parser("verify", help="run the gates without recording an attempt")
    s.add_argument("addr"); s.add_argument("--file"); s.add_argument("--cases", type=int)
    s.add_argument("--json", action="store_true"); s.set_defaults(fn=cmd_verify)
    sub.add_parser("export", help="(lead) write progress/ snapshots").set_defaults(fn=cmd_export)
    s = sub.add_parser("regress", help="re-verify every accepted function")
    s.add_argument("--cases", type=int, default=100); s.set_defaults(fn=cmd_regress)
    s = sub.add_parser("fpucw", help="(lead) set the oracle's x87 control word"); s.add_argument("value")
    s.set_defaults(fn=cmd_fpucw)
    s = sub.add_parser("ghidra", help="(lead) optional Ghidra pseudocode export/import")
    s.add_argument("action", choices=["analyze", "export", "import"]); s.add_argument("--limit", type=int)
    s.set_defaults(fn=cmd_ghidra)
    s = sub.add_parser("runtime", help="(lead) prepare/run the in-game T2 harness")
    s.add_argument("action", choices=["prepare", "install", "uninstall", "coverage-import"])
    s.add_argument("arg", nargs="?"); s.set_defaults(fn=cmd_runtime)
    return p


def main(argv=None):
    a = build_parser().parse_args(argv)
    a.fn(a)
