"""The work ledger: the per-function loop shared by workers and the lead.

Worker loop (see prompts/worker.md):
    claim -> show (context packet) -> write build/scratch/<ADDR>/vK.cpp -> try -> accept | defer

Lead tools: status, sweep, requeue, exclude/include, names conflicts/set, goals, export.
Every command prints compact text for agents; ``--json`` prints machine-readable output.
"""
from __future__ import annotations

import collections
import hashlib
import json
import os
import re
import shutil
import time
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from . import db as dbm
from . import verify
from .analyze import Analyzer
from .paths import REPO, progress_dir
from .spec import parse as parse_spec

LEASE_S = 2 * 3600
DEFAULT_CAP = 8
SCRATCH = REPO / "build" / "scratch"
DONE = dbm.DONE_STATES


def agent_id(args) -> str:
    return getattr(args, "agent", None) or os.environ.get("GP4RE_AGENT") or "lead"


def parse_addr(s: str) -> int:
    return int(s, 16) if not s.lower().startswith("0x") else int(s, 0)


def hx(a: int) -> str:
    return f"0x{a:08x}"


# ===================================================================== claim
def claim(con, agent: str, count: int = 1, min_size: int | None = None, max_size: int | None = None,
          track: str | None = None, region: str | None = None, goal: str | None = None,
          addr: int | None = None, tag: str | None = None) -> list[int]:
    now = time.time()
    goal = goal if goal is not None else dbm.meta_get(con, "active_goal")
    where = ["(l.state='todo' OR (l.state='claimed' AND l.lease_until < :now))", "f.kind='game'"]
    p = {"now": now}
    if addr is not None:
        where = ["l.addr=:addr", "(l.state IN ('todo','deferred') OR (l.state='claimed' AND l.lease_until < :now))"]
        p["addr"] = addr
    if min_size is not None:
        where.append("f.size >= :mn"); p["mn"] = min_size
    if max_size is not None:
        where.append("f.size <= :mx"); p["mx"] = max_size
    if track:
        where.append("l.track = :tr"); p["tr"] = track
    if region:
        where.append("f.region LIKE :rg"); p["rg"] = region + "%"
    if tag:
        where.append("(',' || f.tags || ',') LIKE :tag"); p["tag"] = f"%,{tag},%"
    if goal and addr is None:
        where.append("l.addr IN (SELECT addr FROM goals WHERE goal=:goal)"); p["goal"] = goal
    sql = f"""
      SELECT l.addr,
        (SELECT COUNT(*) FROM edges e JOIN ledger c ON c.addr=e.dst
          WHERE e.src=l.addr AND c.state NOT IN ('accepted','accepted-runtime','excluded')) AS pending
      FROM ledger l JOIN functions f ON f.addr=l.addr
      WHERE {' AND '.join(where)}
      ORDER BY l.priority DESC, pending > 0, f.size, l.addr
      LIMIT :n"""
    p["n"] = count
    got = []
    with dbm.tx(con):
        for a in [r["addr"] for r in con.execute(sql, p)]:
            # re-check under the lock (Thief3 lesson: never hand out a just-accepted function)
            st = con.execute("SELECT state, owner, lease_until FROM ledger WHERE addr=?", (a,)).fetchone()
            if st["state"] in DONE or (st["state"] == "claimed" and (st["lease_until"] or 0) >= now):
                continue
            con.execute("UPDATE ledger SET state='claimed', owner=?, lease_until=?, updated=? WHERE addr=?",
                        (agent, now + LEASE_S, now, a))
            dbm.log_event(con, a, agent, "claim")
            got.append(a)
    return got


def release(con, addr: int, agent: str, note: str | None = None) -> None:
    with dbm.tx(con):
        con.execute("UPDATE ledger SET state='todo', owner=NULL, lease_until=NULL, updated=? "
                    "WHERE addr=? AND state='claimed'", (time.time(), addr))
        dbm.log_event(con, addr, agent, "release", note)


# ===================================================================== context packet
_md = Cs(CS_ARCH_X86, CS_MODE_32)


def _analyzer(con, b) -> Analyzer:
    a = Analyzer(b, log=lambda *x: None)
    a.starts = {r[0] for r in con.execute("SELECT addr FROM functions")}
    return a


def listing(con, b, addr: int, limit: int | None = None) -> list[str]:
    a = _analyzer(con, b)
    f = a.explore(addr)
    names = {}

    def nm(x):
        if x not in names:
            names[x] = dbm.name_of(con, x)
        return names[x]

    out = []
    for i, va in enumerate(sorted(f.insns)):
        if limit and i >= limit:
            out.append(f"  ... {len(f.insns) - limit} more instructions (use --asm for all)")
            break
        ins = f.insns[va]
        note = []
        for tok in re.findall(r"0x[0-9a-f]+", ins.ops):
            v = int(tok, 16)
            if v in b.imports:
                note.append(b.imports[v])
            elif a.in_text(v) and v in a.starts:
                st = con.execute("SELECT state FROM ledger WHERE addr=?", (v,)).fetchone()
                note.append(f"{nm(v)}" + (f" [{st['state']}]" if st else ""))
            elif b.in_image(v) and not a.in_text(v):
                s = b.cstring(v)
                note.append(f'"{s[:60]}"' if s else nm(v))
        out.append(f"  {va:08x}  {ins.mn} {ins.ops}" + (f"    ; {', '.join(note)}" if note else ""))
    return out


def suggest_spec(f, ret_hint: str = "i32") -> str:
    regs = [r for r in (f["regs_in"] or "").split(",") if r]
    bases = json.loads(f["reg_bases"] or "{}")
    if f["style"] == "asm-like":
        args = []
        for r in regs:
            if r in bases:
                args.append(f"{r}:ptr[{hex(min(max(bases[r], 0x40), 0x4000))}]:bytes")
            else:
                args.append(f"{r}:u32")
        if f["fpu_in"]:
            args.append("st0:f64")
        return f"regs({', '.join(args)}) -> (<outputs: registers/st/cf/zf the callers consume>)"
    n = f["args_est"] or ((f["ret_pop"] or 0) // 4)
    conv = {"thiscall": "thiscall", "fastcall": "fastcall"}.get(f["style"]) or \
        ("stdcall" if (f["ret_pop"] or 0) > 0 else "cdecl")
    args = ["ptr[0x100]:bytes"] if conv == "thiscall" else []
    if conv == "thiscall" and f["ret_pop"]:
        n = f["ret_pop"] // 4
    args += ["i32"] * n
    return f"{conv}({', '.join(args)}) -> {ret_hint}"


def call_sites(con, b, callee: int, limit: int = 3) -> list[str]:
    out = []
    callers = [r[0] for r in con.execute("SELECT src FROM edges WHERE dst=? LIMIT ?", (callee, limit))]
    a = _analyzer(con, b)
    for c in callers:
        f = a.explore(c)
        ins = [f.insns[v] for v in sorted(f.insns)]
        for k, i in enumerate(ins):
            if i.mn in ("call", "jmp") and i.ops == hex(callee):
                lo, hi = max(0, k - 4), min(len(ins), k + 7)
                out.append(f"  in {dbm.name_of(con, c)} ({hx(c)}):")
                out += [f"    {'>' if j == k else ' '} {ins[j].va:08x}  {ins[j].mn} {ins[j].ops}" for j in range(lo, hi)]
                break
    return out


def packet(con, addr: int, full_asm: bool = False) -> str:
    b = verify.gp4_binary()
    f = con.execute("SELECT * FROM functions WHERE addr=?", (addr,)).fetchone()
    if not f:
        return f"{hx(addr)}: not a known function start"
    l = con.execute("SELECT * FROM ledger WHERE addr=?", (addr,)).fetchone()
    L = []
    L.append(f"== {hx(addr)} {dbm.name_of(con, addr)}  size={f['size']} insns={f['n_insn']} branches={f['n_cond']} "
             f"x87={f['x87']}  track={l['track']} style={f['style']} region={f['region']} kind={f['kind']} "
             f"state={l['state']} attempts={l['attempts']}" + (f" tags={f['tags']}" if f["tags"] else ""))
    ins = []
    if f["regs_in"]:
        ins.append(f"registers consumed on entry: {f['regs_in']}")
    if f["flags_in"]:
        ins.append("reads EFLAGS set by the caller")
    if f["fpu_in"]:
        ins.append("consumes values already on the x87 stack")
    if f["has_frame"]:
        ins.append(f"ebp frame, ~{f['args_est']} stack args")
    if f["ret_pop"]:
        ins.append(f"ret {f['ret_pop']} (callee pops {f['ret_pop'] // 4} dwords)")
    if ins:
        L.append("   " + "; ".join(ins))
    L.append(f"   suggested spec: {suggest_spec(f)}")
    nm = con.execute("SELECT * FROM names WHERE addr=?", (addr,)).fetchone()
    if nm:
        L.append(f"   name: {nm['name']} {nm['type'] or ''} ({nm['conf']}, {nm['source']}: {nm['evidence'] or ''})")
    for label, sql in (("callers", "SELECT src AS a FROM edges WHERE dst=? LIMIT 12"),
                       ("callees", "SELECT dst AS a FROM edges WHERE src=?")):
        xs = [r["a"] for r in con.execute(sql, (addr,))]
        if xs:
            parts = []
            for x in xs:
                st = con.execute("SELECT state FROM ledger WHERE addr=?", (x,)).fetchone()
                parts.append(f"{hx(x)} {dbm.name_of(con, x)}" + (f" [{st['state']}]" if st and label == "callees" else ""))
            L.append(f"   {label} ({len(xs)}): " + ", ".join(parts))
    imps = [r[0] for r in con.execute("SELECT import FROM func_imports WHERE addr=?", (addr,))]
    if imps:
        slots = {v: k for k, v in b.imports.items()}
        L.append("   imports (call through GP4_IAT(type, slot)): " +
                 ", ".join(f"{i} @{slots.get(i, 0):#x}" for i in imps))
    gl = [r[0] for r in con.execute("SELECT gaddr FROM func_globals WHERE addr=? ORDER BY gaddr", (addr,))]
    if gl:
        parts = []
        for g in gl[:40]:
            s = b.cstring(g)
            parts.append(f'{hx(g)} "{s[:50]}"' if s else f"{hx(g)} {dbm.name_of(con, g)}")
        L.append(f"   globals ({len(gl)}): " + ", ".join(parts))
    twins = [r[0] for r in con.execute("SELECT addr FROM functions WHERE norm_hash=? AND addr!=? LIMIT 8",
                                       (f["norm_hash"], addr))]
    if twins:
        tw = []
        for t in twins:
            st = con.execute("SELECT state, src FROM ledger WHERE addr=?", (t,)).fetchone()
            tw.append(f"{hx(t)} [{st['state']}{' ' + st['src'] if st['src'] else ''}]")
        L.append("   byte-identical twins (modulo addresses): " + ", ".join(tw))
    if l["why"]:
        L.append(f"   previous deferral: {l['why']}" + (f" | needs: {l['needs']}" if l["needs"] else ""))
    for a in con.execute("SELECT * FROM attempts WHERE addr=? ORDER BY id DESC LIMIT 3", (addr,)):
        s = json.loads(a["summary"] or "{}")
        L.append(f"   attempt by {a['agent']} ({a['file']}): {a['verdict']} {s.get('why', '')[:200]}")
    ps = con.execute("SELECT * FROM pseudocode WHERE addr=?", (addr,)).fetchone()
    if ps:
        L.append(f"--- {ps['tool']} pseudocode ({ps['signature']})")
        L += ["  " + x for x in ps["code"].splitlines()[:120]]
    if f["style"] == "asm-like":
        cs = call_sites(con, b, addr)
        if cs:
            L.append("--- call sites (what the caller does with the outputs)")
            L += cs
    L.append("--- disassembly")
    L += listing(con, b, addr, None if full_asm else 400)
    nb = con.execute("SELECT l.addr, l.src FROM ledger l JOIN functions f ON f.addr=l.addr WHERE f.region=? "
                     "AND l.state IN ('accepted','accepted-runtime') ORDER BY ABS(l.addr-?) LIMIT 1",
                     (f["region"], addr)).fetchone()
    if nb and nb["src"] and (REPO / nb["src"]).exists():
        src = (REPO / nb["src"]).read_text(errors="replace").splitlines()
        L.append(f"--- nearest accepted function in this region: {nb['src']}")
        L += ["  " + x for x in src[:45]]
    return "\n".join(L)


# ===================================================================== try / accept / defer
def _cand_path(addr: int, file: str) -> Path:
    p = Path(file)
    return p if p.is_absolute() else (REPO / p)


def try_candidate(con, addr: int, file: str, agent: str, cap: int, record: bool = True) -> dict:
    cand = _cand_path(addr, file)
    if not cand.exists():
        return {"overall": "error", "why": f"{file} does not exist"}
    src_sha = hashlib.sha1(cand.read_bytes()).hexdigest()[:16]
    prev = con.execute("SELECT id, file FROM attempts WHERE addr=? AND src_sha=?", (addr, src_sha)).fetchone()
    if prev and record:
        return {"overall": "refused", "why": f"byte-identical to an earlier attempt ({prev['file']}); change something"}
    res = verify.run(addr, candidate=cand, agent=agent)
    counted = not any(g["gate"] == "build" and g["status"] == "fail" for g in res["gates"])
    res["why"] = _why(res)
    if record:
        n_prev = con.execute("SELECT COUNT(*) FROM attempts WHERE addr=? AND verdict!='build-failed'", (addr,)).fetchone()[0]
        res["attempt"] = n_prev + (1 if counted else 0)
        res["cap"] = cap
        same = None
        if res.get("code_sha"):
            same = con.execute("SELECT file FROM attempts WHERE addr=? AND code_sha=? ORDER BY id LIMIT 1",
                               (addr, res["code_sha"])).fetchone()
        if same:
            res["same_code_as"] = same["file"]
        verdict = res["overall"] if counted else "build-failed"
        with dbm.tx(con):
            con.execute("INSERT INTO attempts(addr, agent, ts, file, src_sha, code_sha, verdict, summary) "
                        "VALUES(?,?,?,?,?,?,?,?)",
                        (addr, agent, time.time(), _rel(cand), src_sha, res.get("code_sha"), verdict,
                         json.dumps({"why": res["why"], "gates": _brief(res)})))
            if counted:
                con.execute("UPDATE ledger SET attempts=attempts+1, updated=? WHERE addr=?", (time.time(), addr))
    return res


def accept(con, addr: int, file: str, agent: str) -> dict:
    cand = _cand_path(addr, file)
    res = verify.run(addr, candidate=cand, agent=agent)
    res["why"] = _why(res)
    if res["overall"] not in ("pass", "pass-pending-runtime"):
        return {"accepted": False, **res}
    l = con.execute("SELECT track FROM ledger WHERE addr=?", (addr,)).fetchone()
    dest = REPO / "src" / "gp4" / l["track"] / f"{addr:08x}"[:4] / f"{addr:08x}.cpp"
    dest.parent.mkdir(parents=True, exist_ok=True)
    for old in verify.msvc.sources(REPO):           # one home per function
        if old.resolve() != dest.resolve() and verify.impl_pattern(addr).search(old.read_text(errors="replace")):
            old.unlink()
    shutil.copyfile(cand, dest)
    state = dbm.ACCEPTED if res["overall"] == "pass" else dbm.ACCEPTED_RT
    with dbm.tx(con):
        con.execute("UPDATE ledger SET state=?, owner=?, lease_until=NULL, src=?, best=?, why=NULL, needs=NULL, "
                    "updated=? WHERE addr=?",
                    (state, agent, _rel(dest), json.dumps(_brief(res)), time.time(), addr))
        if res.get("name") and not re.fullmatch(r"FUN_[0-9a-fA-F]{8}", res["name"]):
            _propose_name(con, addr, res["name"], None, agent, "name used by the accepted reconstruction", "func")
        dbm.log_event(con, addr, agent, "accept", {"state": state, "src": _rel(dest)})
    return {"accepted": True, "state": state, "dest": _rel(dest), **res}


def defer(con, addr: int, agent: str, why: str, needs: str | None) -> None:
    with dbm.tx(con):
        con.execute("UPDATE ledger SET state='deferred', owner=?, lease_until=NULL, why=?, needs=?, updated=? "
                    "WHERE addr=?", (agent, why, needs, time.time(), addr))
        dbm.log_event(con, addr, agent, "defer", {"why": why, "needs": needs})


def _rel(p: Path) -> str:
    try:
        return p.resolve().relative_to(REPO.resolve()).as_posix()
    except ValueError:
        return str(p)


def _brief(res: dict) -> dict:
    out = {}
    for g in res.get("gates", []):
        k = g["gate"]
        if k == "emu":
            out[k] = {x: g.get(x) for x in ("status", "passed", "invalid", "cases", "mismatch", "reason", "coverage", "non_interference") if g.get(x) is not None}
        elif k == "shape":
            out[k] = {"tier": g.get("tier"), "missing_callees": g.get("missing_callees"),
                      "extra_callees": g.get("extra_callees"), "globals_jaccard": g.get("globals_jaccard")}
        else:
            out[k] = g["status"]
    return out


def _why(res: dict) -> str:
    for g in res.get("gates", []):
        if g["status"] == "fail":
            if g["gate"] in ("lint", "registry", "spec"):
                return f"{g['gate']}: " + "; ".join(g.get("problems", []))
            if g["gate"] == "build":
                return f"build: {g.get('file')}: {g.get('log', '')[-600:]}"
            if g["gate"] == "shape":
                return (f"shape tier C: missing callees {g['missing_callees']} extra {g['extra_callees']} "
                        f"imports -{g['missing_imports']} +{g['extra_imports']} globals J={g['globals_jaccard']} "
                        f"missing {g['missing_globals'][:6]} extra {g['extra_globals'][:6]}")
            if g["gate"] == "emu":
                if g.get("reason"):
                    return "emu: " + g["reason"] + (f" ({g['problems']})" if g.get("problems") else "")
                return f"emu case {g.get('case')}: inputs {g.get('inputs')} -> {g.get('mismatch')}"
    e = next((g for g in res.get("gates", []) if g["gate"] == "emu"), {})
    if e.get("status") == "inconclusive":
        if e.get("reason"):
            return "emu inconclusive: " + e["reason"]
        return f"emu inconclusive: {e.get('invalid')} of {e.get('cases')} cases invalid for the ORIGINAL " \
               f"({e.get('invalid_reasons')}) - improve the spec (pointer sizes/fills, ranges)"
    return ""


def render_verdict(res: dict) -> str:
    if res.get("overall") in ("refused", "error"):
        return f"{res['overall'].upper()}: {res['why']}"
    L = []
    if "attempt" in res:
        L.append(f"ATTEMPT {res['attempt']}/{res['cap']}  {res['addr']} {res.get('name', '')}")
    for g in res["gates"]:
        k, s = g["gate"], g["status"]
        if k == "shape":
            L.append(f"shape  {s.upper()} tier {g['tier']}  callees -{g['missing_callees']} +{g['extra_callees']}  "
                     f"imports -{g['missing_imports']} +{g['extra_imports']}  globals J={g['globals_jaccard']}"
                     f"  x87 {g['x87']['original']}/{g['x87']['reconstruction']}"
                     f"  branches {g['branches']['original']}/{g['branches']['reconstruction']}")
        elif k == "emu":
            if s == "pass":
                L.append(f"emu    PASS {g['passed']}/{g['cases'] - g['invalid']} valid cases"
                         + (f" ({g['invalid']} invalid for the original)" if g["invalid"] else ""))
            elif s == "fail":
                if "case" in g:
                    L.append(f"emu    FAIL at case {g['case']} inputs={g.get('inputs')}\n       {g.get('mismatch') or g.get('reason')}")
                else:
                    L.append(f"emu    FAIL {g.get('reason') or g.get('problems')}")
            else:
                L.append(f"emu    {s.upper()} {g.get('reason') or g.get('invalid_reasons')}")
            coverage = g.get("coverage")
            if coverage:
                L.append(f"       coverage: {coverage}")
            for w in g.get("warnings", []):
                L.append(f"       warning: {w}")
        elif k == "build" and s == "fail":
            L.append(f"BUILD FAILED ({g.get('file')}) - not counted as an attempt\n{g.get('log', '')[-2500:]}")
        else:
            probs = g.get("problems")
            L.append(f"{k:6} {s.upper()}" + (f"  {probs}" if probs else ""))
    if res.get("same_code_as"):
        L.append(f"note: same compiled code as {res['same_code_as']} - that change did nothing")
    if res.get("field_macros"):
        L.append(f"note: {res['field_macros']} GP4_FIELD uses - declare a struct when the layout is clear")
    tag = {"pass": "PASS", "pass-pending-runtime": "PASS (pending in-game check)", "fail": "NO MATCH"}.get(
        res["overall"], res["overall"])
    L.append(f"VERDICT: {tag}" + (f" - {res['why']}" if res["overall"] == "fail" and res.get("why") else ""))
    return "\n".join(L)


# ===================================================================== names
def _propose_name(con, addr, name, typ, agent, evidence, kind="func"):
    cur = con.execute("SELECT name FROM names WHERE addr=?", (addr,)).fetchone()
    if cur is None:
        con.execute("INSERT INTO names VALUES(?,?,?,?,?,?,?,?)",
                    (addr, name, kind, typ, "provisional", agent, evidence, time.time()))
        return "set"
    if cur["name"] == name:
        if typ:
            con.execute("UPDATE names SET type=COALESCE(type, ?) WHERE addr=?", (typ, addr))
        return "same"
    con.execute("INSERT INTO names_proposed(addr, name, type, agent, evidence, ts) VALUES(?,?,?,?,?,?)",
                (addr, name, typ, agent, evidence, time.time()))
    return f"conflict with '{cur['name']}' - recorded for the lead's naming pass"


def name(con, addr, nm, typ, agent, evidence, kind, force=False):
    with dbm.tx(con):
        if force:
            con.execute("INSERT OR REPLACE INTO names VALUES(?,?,?,?,?,?,?,?)",
                        (addr, nm, kind, typ, "confirmed", agent, evidence, time.time()))
            con.execute("UPDATE names_proposed SET resolved=1 WHERE addr=?", (addr,))
            return "set (confirmed)"
        return _propose_name(con, addr, nm, typ, agent, evidence, kind)


# ===================================================================== lead tools
def status(con) -> dict:
    by_state = collections.Counter()
    bytes_state = collections.Counter()
    by_track = collections.defaultdict(collections.Counter)
    for r in con.execute("SELECT l.state, l.track, f.size, f.kind FROM ledger l JOIN functions f ON f.addr=l.addr"):
        by_state[r["state"]] += 1
        bytes_state[r["state"]] += r["size"]
        if r["kind"] == "game":
            by_track[r["track"]][r["state"]] += 1
    game_bytes = con.execute("SELECT SUM(size) FROM functions WHERE kind='game'").fetchone()[0] or 1
    done_bytes = sum(bytes_state[s] for s in DONE)
    return {
        "functions": sum(by_state.values()), "by_state": dict(by_state),
        "by_track": {k: dict(v) for k, v in by_track.items()},
        "game_bytes": game_bytes, "accepted_bytes": done_bytes,
        "progress_pct": round(100 * done_bytes / game_bytes, 3),
        "active_goal": dbm.meta_get(con, "active_goal"),
        "names": con.execute("SELECT COUNT(*) FROM names").fetchone()[0],
        "name_conflicts": con.execute("SELECT COUNT(*) FROM names_proposed WHERE resolved=0").fetchone()[0],
        "fpucw": hex(dbm.meta_get(con, "fpucw") or __import__("gp4re.config", fromlist=["oracle"]).oracle("fpu_cw")),
    }


def sweep(con, since_h: float) -> dict:
    t0 = time.time() - since_h * 3600
    per = collections.defaultdict(collections.Counter)
    reasons = collections.Counter()
    needs = collections.Counter()
    for e in con.execute("SELECT * FROM events WHERE ts>=?", (t0,)):
        per[e["agent"]][e["event"]] += 1
        if e["event"] == "defer" and e["detail"]:
            d = json.loads(e["detail"])
            reasons[re.sub(r"0x[0-9a-f]+", "X", (d.get("why") or "")[:60])] += 1
            if d.get("needs"):
                needs[d["needs"][:120]] += 1
    att = con.execute("SELECT verdict, COUNT(*) n FROM attempts WHERE ts>=? GROUP BY verdict", (t0,)).fetchall()
    return {"since_hours": since_h, "agents": {k: dict(v) for k, v in per.items()},
            "attempt_verdicts": {r["verdict"]: r["n"] for r in att},
            "top_deferral_reasons": reasons.most_common(15), "needs": needs.most_common(25),
            "open_name_conflicts": con.execute("SELECT COUNT(*) FROM names_proposed WHERE resolved=0").fetchone()[0]}


def goal_closure(con, goal: str, root: int, depth: int) -> int:
    seen, frontier = {root}, [root]
    for _ in range(depth):
        nxt = []
        for a in frontier:
            for (d,) in con.execute("SELECT dst FROM edges WHERE src=?", (a,)):
                if d not in seen:
                    seen.add(d)
                    nxt.append(d)
        frontier = nxt
    with dbm.tx(con):
        con.executemany("INSERT OR IGNORE INTO goals VALUES(?,?)", [(goal, a) for a in seen])
    return len(seen)


def goal_import(con, goal: str, path: Path) -> int:
    addrs = {int(m, 16) for m in re.findall(r"0x[0-9a-fA-F]{6,8}", path.read_text(errors="replace"))}
    known = {r[0] for r in con.execute("SELECT addr FROM functions")}
    hit = sorted(addrs & known)
    with dbm.tx(con):
        con.executemany("INSERT OR IGNORE INTO goals VALUES(?,?)", [(goal, a) for a in hit])
    return len(hit)


def export(con) -> list[str]:
    pd = progress_dir()
    st = status(con)
    (pd / "report.json").write_text(json.dumps(st, indent=2, sort_keys=True) + "\n")
    lines = ["# addr name kind conf type  (gp4re export; source of truth is the shared ledger DB)"]
    for r in con.execute("SELECT * FROM names ORDER BY addr"):
        lines.append(f"{hx(r['addr'])} {r['name']} {r['kind']} {r['conf']} {r['type'] or ''}".rstrip())
    (pd / "symbols.txt").write_text("\n".join(lines) + "\n")
    acc = ["# addr state src"]
    for r in con.execute("SELECT addr, state, src FROM ledger WHERE state IN ('accepted','accepted-runtime',"
                         "'deferred','excluded') ORDER BY addr"):
        acc.append(f"{hx(r['addr'])} {r['state']} {r['src'] or ''}".rstrip())
    (pd / "ledger.txt").write_text("\n".join(acc) + "\n")
    return ["progress/report.json", "progress/symbols.txt", "progress/ledger.txt"]
