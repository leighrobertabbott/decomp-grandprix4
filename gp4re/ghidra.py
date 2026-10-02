"""Ghidra pseudocode for the context packets (``gp4re ghidra analyze|export|import``).

Ghidra's decompiler output is extra context for workers - a rough C rendering of
the function they are rebuilding, the way Thief3-Decomp's packets carry a cached
Ghidra decompile. It is never used by any acceptance check.

  analyze   one-time headless import + auto-analysis of GP4.exe (tens of minutes)
  export    decompile the queued game functions (core region first) and import them
  import    load analysis/ghidra_decomp.jsonl into the database (safe while export runs)

Ghidra is found via GHIDRA_INSTALL_DIR or [ghidra] dir in config/project.toml.
"""
from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path

from . import db as dbm
from .config import cfg
from .paths import REPO, analysis_dir, find_exe, main_root

REGION_ORDER = ("R1", "R3", "R2", "R5", "R6")


def ghidra_dir() -> Path:
    env = os.environ.get("GHIDRA_INSTALL_DIR")
    if env:
        return Path(env)
    conf = cfg().get("ghidra", {}).get("dir")
    if conf and (main_root() / conf).exists():
        return main_root() / conf
    for p in sorted((main_root() / "toolchain").glob("ghidra_*_PUBLIC"), reverse=True):
        return p
    raise SystemExit("Ghidra not found: set GHIDRA_INSTALL_DIR or [ghidra] dir in config/project.toml")


def _headless() -> Path:
    p = ghidra_dir() / "support" / "analyzeHeadless.bat"
    if not p.exists():
        raise SystemExit(f"{p} not found")
    return p


def _project() -> Path:
    p = analysis_dir() / "ghidra_project"
    p.mkdir(exist_ok=True)
    return p


def _env() -> dict:
    env = dict(os.environ)
    env.setdefault("GHIDRA_HEADLESS_MAXMEM", "4G")
    return env


def analyze() -> None:
    proj = _project()
    if (proj / "GP4.gpr").exists():
        print(f"Ghidra project already exists: {proj}")
        return
    cmd = [str(_headless()), str(proj), "GP4", "-import", str(find_exe()), "-analysisTimeoutPerFile", "7200"]
    print(" ".join(cmd))
    subprocess.run(cmd, check=True, env=_env())


def starts(limit: int | None) -> list[int]:
    con = dbm.connect()
    rows = con.execute("SELECT f.addr, f.region FROM functions f JOIN ledger l ON l.addr=f.addr "
                       "WHERE f.kind='game' ORDER BY f.addr").fetchall()

    def rank(r):
        reg = (r["region"] or "")[:2]
        return (REGION_ORDER.index(reg) if reg in REGION_ORDER else len(REGION_ORDER), r["addr"])
    out = [r["addr"] for r in sorted(rows, key=rank)]
    return out[:limit] if limit else out


def export(limit: int | None) -> Path:
    proj = _project()
    if not (proj / "GP4.gpr").exists():
        analyze()
    ad = analysis_dir()
    sf = ad / "ghidra_starts.txt"
    sf.write_text("\n".join(f"{a:08x}" for a in starts(limit)) + "\n")
    out = ad / "ghidra_decomp.jsonl"
    cmd = [str(_headless()), str(proj), "GP4", "-process", "GP4.exe", "-noanalysis",
           "-scriptPath", str(REPO / "tools" / "ghidra"), "-postScript", "ExportDecomp.java", str(sf), str(out)]
    print(" ".join(cmd))
    subprocess.run(cmd, check=True, env=_env())
    return out


def import_(path: Path | None = None) -> int:
    path = path or analysis_dir() / "ghidra_decomp.jsonl"
    con = dbm.connect()
    n = 0
    with dbm.tx(con):
        for line in path.read_text(encoding="utf-8").splitlines():
            try:
                d = json.loads(line)
            except json.JSONDecodeError:
                continue            # a line still being written
            con.execute("INSERT OR REPLACE INTO pseudocode VALUES(?,?,?,?)",
                        (d["addr"], "ghidra", d["signature"], d["c"]))
            n += 1
    return n


def main(a) -> None:
    if a.action == "analyze":
        analyze()
    elif a.action == "export":
        out = export(a.limit)
        print(f"imported {import_(out)} pseudocode entries")
    else:
        print(f"imported {import_()} pseudocode entries")
