"""SQLite store shared by the lead and every worker (WAL; claims are atomic).

Analysis tables are rebuilt by ``gp4re analyze``; knowledge (names, regions,
goals), the ledger and attempt history persist across re-analysis.
"""
from __future__ import annotations

import json
import math
import sqlite3
import time
from contextlib import contextmanager

from .paths import db_path

SCHEMA = """
CREATE TABLE IF NOT EXISTS meta(key TEXT PRIMARY KEY, value TEXT);

CREATE TABLE IF NOT EXISTS functions(
  addr INTEGER PRIMARY KEY, end_addr INTEGER, size INTEGER, n_insn INTEGER, n_cond INTEGER,
  x87 INTEGER, mmx INTEGER, sse INTEGER, has_frame INTEGER, frame_size INTEGER, args_est INTEGER,
  ret_pop INTEGER, indirect_calls INTEGER, indirect_jumps INTEGER, invalid INTEGER, source TEXT,
  style TEXT, regs_in TEXT, flags_in INTEGER, fpu_in INTEGER, difficulty REAL,
  kind TEXT, kind_evidence TEXT, region TEXT, norm_hash TEXT, reg_bases TEXT, tags TEXT);
CREATE TABLE IF NOT EXISTS edges(src INTEGER, dst INTEGER, kind TEXT, PRIMARY KEY(src, dst, kind));
CREATE INDEX IF NOT EXISTS edges_dst ON edges(dst);
CREATE TABLE IF NOT EXISTS func_imports(addr INTEGER, import TEXT, PRIMARY KEY(addr, import));
CREATE TABLE IF NOT EXISTS func_globals(addr INTEGER, gaddr INTEGER, PRIMARY KEY(addr, gaddr));
CREATE INDEX IF NOT EXISTS func_globals_g ON func_globals(gaddr);
CREATE TABLE IF NOT EXISTS strings(gaddr INTEGER PRIMARY KEY, text TEXT);
CREATE TABLE IF NOT EXISTS jtables(addr INTEGER, table_addr INTEGER, n INTEGER);
CREATE TABLE IF NOT EXISTS pseudocode(addr INTEGER PRIMARY KEY, tool TEXT, signature TEXT, code TEXT);

CREATE TABLE IF NOT EXISTS names(
  addr INTEGER PRIMARY KEY, name TEXT, kind TEXT, type TEXT, conf TEXT, source TEXT,
  evidence TEXT, ts REAL);
CREATE INDEX IF NOT EXISTS names_name ON names(name);
CREATE TABLE IF NOT EXISTS names_proposed(
  id INTEGER PRIMARY KEY AUTOINCREMENT, addr INTEGER, name TEXT, type TEXT, agent TEXT,
  evidence TEXT, ts REAL, resolved INTEGER DEFAULT 0);
CREATE TABLE IF NOT EXISTS regions(
  start INTEGER, end_addr INTEGER, label TEXT, kind TEXT, exclude INTEGER, confidence TEXT, note TEXT,
  PRIMARY KEY(start, end_addr));

CREATE TABLE IF NOT EXISTS ledger(
  addr INTEGER PRIMARY KEY, state TEXT, track TEXT, owner TEXT, lease_until REAL,
  attempts INTEGER DEFAULT 0, priority INTEGER DEFAULT 0, best TEXT, why TEXT, needs TEXT,
  src TEXT, updated REAL);
CREATE INDEX IF NOT EXISTS ledger_state ON ledger(state);
CREATE TABLE IF NOT EXISTS attempts(
  id INTEGER PRIMARY KEY AUTOINCREMENT, addr INTEGER, agent TEXT, ts REAL, file TEXT,
  src_sha TEXT, code_sha TEXT, verdict TEXT, summary TEXT);
CREATE INDEX IF NOT EXISTS attempts_addr ON attempts(addr);
CREATE TABLE IF NOT EXISTS events(
  id INTEGER PRIMARY KEY AUTOINCREMENT, ts REAL, addr INTEGER, agent TEXT, event TEXT, detail TEXT);
CREATE TABLE IF NOT EXISTS goals(goal TEXT, addr INTEGER, PRIMARY KEY(goal, addr));
"""

ANALYSIS_TABLES = ("functions", "edges", "func_imports", "func_globals", "strings", "jtables")
LIB_KINDS = ("crt", "d3dx", "zlib", "libpng", "compiler", "lib?")
# ledger states
TODO, CLAIMED, ACCEPTED, ACCEPTED_RT, DEFERRED, EXCLUDED, ORPHANED = (
    "todo", "claimed", "accepted", "accepted-runtime", "deferred", "excluded", "orphaned")
DONE_STATES = (ACCEPTED, ACCEPTED_RT)


def connect() -> sqlite3.Connection:
    con = sqlite3.connect(db_path(), timeout=60, isolation_level=None)
    con.row_factory = sqlite3.Row
    con.execute("PRAGMA journal_mode=WAL")
    con.execute("PRAGMA busy_timeout=60000")
    con.execute("PRAGMA synchronous=NORMAL")
    con.executescript(SCHEMA)
    return con


@contextmanager
def tx(con: sqlite3.Connection):
    """Write transaction that takes the lock up front, so two workers can never claim the same row."""
    for i in range(60):
        try:
            con.execute("BEGIN IMMEDIATE")
            break
        except sqlite3.OperationalError:
            time.sleep(0.25 * (i + 1))
    else:
        raise RuntimeError("database is locked")
    try:
        yield con
        con.execute("COMMIT")
    except BaseException:
        con.execute("ROLLBACK")
        raise


def meta_get(con, key, default=None):
    r = con.execute("SELECT value FROM meta WHERE key=?", (key,)).fetchone()
    return json.loads(r["value"]) if r else default


def meta_set(con, key, value):
    con.execute("INSERT OR REPLACE INTO meta(key, value) VALUES(?, ?)", (key, json.dumps(value)))


def difficulty(r: dict) -> float:
    d = math.log2(r["n_insn"] + 1) + 0.6 * math.log2(r["n_cond"] + 1)
    d += 0.4 * math.log2(r["x87"] + 1) + 0.3 * math.log2(len(r["calls"]) + 1)
    d += 2.0 * r["indirect_jumps"] + (1.5 if r["style"] == "asm-like" else 0)
    return round(d, 2)


def track_for(r: dict) -> str:
    return "asm" if r["style"] == "asm-like" else "c"


def load_analysis(con, rows: list[dict], stats: dict, fingerprint: dict, kinds: dict,
                  regions: list[dict]) -> dict:
    """Replace analysis tables; add ledger rows for new functions; flag vanished ones."""
    def region_of(a):
        return next((g for g in regions if g["start"] <= a < g["end"]), None)

    with tx(con):
        for t in ANALYSIS_TABLES:          # rebuilt with the current schema every time
            con.execute(f"DROP TABLE IF EXISTS {t}")
        for stmt in SCHEMA.split(";"):
            if any(f"EXISTS {t}(" in stmt or f"ON {t}(" in stmt for t in ANALYSIS_TABLES):
                con.execute(stmt)
        con.execute("DELETE FROM regions")
        for g in regions:
            con.execute("INSERT INTO regions VALUES(?,?,?,?,?,?,?)",
                        (g["start"], g["end"], g["label"], g["kind"], int(g["exclude"]), g["confidence"], g["note"]))
        strings = {}
        for r in rows:
            k, ev = kinds[r["addr"]]
            reg = region_of(r["addr"])
            if k == "game" and reg and reg["exclude"]:
                k, ev = "lib?", f"provisional library region {reg['label']}"
            r["kind"] = k
            con.execute(
                "INSERT INTO functions VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                (r["addr"], r["end"], r["size"], r["n_insn"], r["n_cond"], r["x87"], r["mmx"], r["sse"],
                 int(r["has_frame"]), r["frame_size"], r["args_est"], r["ret_pop"], r["indirect_calls"],
                 r["indirect_jumps"], int(r["invalid"]), r["source"], r["style"], ",".join(r["regs_in"]),
                 int(r["flags_in"]), int(r["fpu_in"]), difficulty(r), k, ev, reg["label"] if reg else None,
                 r["norm_hash"], json.dumps(r["reg_bases"]), r.get("tags", "")))
            con.executemany("INSERT OR IGNORE INTO edges VALUES(?,?,?)",
                            [(r["addr"], d, "call") for d in r["calls"]] +
                            [(r["addr"], d, "tail") for d in r["tails"]])
            con.executemany("INSERT OR IGNORE INTO func_imports VALUES(?,?)", [(r["addr"], i) for i in r["imports"]])
            con.executemany("INSERT OR IGNORE INTO func_globals VALUES(?,?)", [(r["addr"], g) for g in r["globals"]])
            con.executemany("INSERT INTO jtables VALUES(?,?,?)", [(r["addr"], t, n) for t, n in r["jtables"]])
            for g, s in r["strings"].items():
                strings[int(g)] = s
        con.executemany("INSERT OR REPLACE INTO strings VALUES(?,?)", strings.items())
        now = time.time()
        known = {row[0]: row[1] for row in con.execute("SELECT addr, state FROM ledger")}
        new = 0
        for r in rows:
            excluded = r["kind"] in LIB_KINDS or r["kind"].endswith("?")
            if r["addr"] not in known:
                con.execute("INSERT INTO ledger(addr, state, track, attempts, priority, why, updated) "
                            "VALUES(?,?,?,0,0,?,?)",
                            (r["addr"], EXCLUDED if excluded else TODO, track_for(r),
                             f"{r['kind']}: {kinds[r['addr']][1]}" if excluded else None, now))
                new += 1
            elif known[r["addr"]] == TODO and excluded:
                con.execute("UPDATE ledger SET state=?, why=?, updated=? WHERE addr=?",
                            (EXCLUDED, f"{r['kind']}", now, r["addr"]))
        present = {r["addr"] for r in rows}
        orphaned = [a for a in known if a not in present]
        for a in orphaned:
            con.execute("UPDATE ledger SET state=?, updated=? WHERE addr=? AND state NOT IN (?,?)",
                        (ORPHANED, now, a, ACCEPTED, ACCEPTED_RT))
        meta_set(con, "analysis_stats", stats)
        meta_set(con, "binary", fingerprint)
        meta_set(con, "analyzed_at", now)
    return {"new_ledger_rows": new, "orphaned": len(orphaned)}


def log_event(con, addr, agent, event, detail=None):
    con.execute("INSERT INTO events(ts, addr, agent, event, detail) VALUES(?,?,?,?,?)",
                (time.time(), addr, agent, event, json.dumps(detail) if detail is not None else None))


def name_of(con, addr: int) -> str:
    r = con.execute("SELECT name FROM names WHERE addr=?", (addr,)).fetchone()
    if r and r["name"]:
        return r["name"]
    f = con.execute("SELECT 1 FROM functions WHERE addr=?", (addr,)).fetchone()
    return f"FUN_{addr:08x}" if f else f"DAT_{addr:08x}"
