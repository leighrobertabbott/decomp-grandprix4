"""In-game harness (verification tier T2) - ``gp4re runtime ...``.

  prepare          copy the install to runtime/GP4 (the original folder is never modified)
  install [all]    build hook/ + every reconstruction into runtime/GP4/dinput8.dll and write
                   gp4hook.ini: accepted functions hooked (or only accepted-runtime ones),
                   coverage armed if GP4RE_COVERAGE=1
  uninstall        remove the proxy DLL from runtime/GP4
  coverage-import  turn runtime/GP4/coverage.log into goals: <prefix>-mark0, -mark1, ...

The game is launched by a human (or a desktop-automation agent) from runtime/GP4
exactly as the user normally launches it; gp4hook.log records what was installed.
"""
from __future__ import annotations

import os
import re
import shutil
from pathlib import Path

from . import db as dbm
from . import msvc
from .paths import REPO, build_dir, find_game_dir, main_root

HOOK_CFLAGS = ["/nologo", "/c", "/O2", "/Oy-", "/arch:IA32", "/fp:precise", "/MT", "/EHsc", "/GS-",
               "/std:c++17", "/W3", "/Zc:threadSafeInit-", "/D_CRT_SECURE_NO_WARNINGS", "/DGP4_HOOK=1"]


def runtime_dir() -> Path:
    return main_root() / "runtime" / "GP4"


def prepare(fresh: bool = False) -> Path:
    src = find_game_dir()
    dst = runtime_dir()
    if dst.exists() and fresh:
        shutil.rmtree(dst)
    if not dst.exists():
        print(f"copying {src} -> {dst} ...")
        shutil.copytree(src, dst)
    return dst


def build_hook() -> Path:
    env = msvc.msvc_env()
    out = build_dir() / "hook"
    out.mkdir(parents=True, exist_ok=True)
    files = [REPO / "hook" / "src" / "gp4hook.cpp", REPO / "hook" / "src" / "adapter.cpp"] + \
            [p for p in msvc.sources(REPO) if p.name != "gp4_freestanding.cpp"]
    objs = []
    for i, f in enumerate(files):
        obj = out / f"{i:05d}_{f.stem}.obj"
        rc, log = msvc._run(["cl", *HOOK_CFLAGS, f"/I{REPO / 'include'}", f"/Fo{obj}", str(f)], env, REPO)
        if rc:
            raise SystemExit(f"hook build failed in {f}:\n{log[-3000:]}")
        objs.append(str(obj))
    dll = out / "dinput8.dll"
    rc, log = msvc._run(["link", "/nologo", "/DLL", f"/DEF:{REPO / 'hook' / 'src' / 'dinput8.def'}",
                         "/OPT:NOREF", "/SAFESEH:NO", f"/OUT:{dll}", *objs, "kernel32.lib", "user32.lib"], env, REPO)
    if rc:
        raise SystemExit(f"hook link failed:\n{log[-3000:]}")
    return dll


def write_ini(rt: Path, only_runtime: bool, coverage: bool) -> dict:
    con = dbm.connect()
    states = ("accepted-runtime",) if only_runtime else ("accepted", "accepted-runtime")
    rows = con.execute(f"SELECT l.addr, f.size, l.state FROM ledger l JOIN functions f ON f.addr=l.addr "
                       f"WHERE l.state IN ({','.join('?' * len(states))}) ORDER BY l.addr", states).fetchall()
    lines = ["; written by gp4re runtime install - edit 1/0 to toggle a reconstruction", "[hooks]"]
    hooked = set()
    for r in rows:
        if r["size"] < 5:
            lines.append(f"; 0x{r['addr']:08x} skipped: original is {r['size']} bytes (< 5-byte jmp patch)")
            continue
        lines.append(f"0x{r['addr']:08x}=1 ; {dbm.name_of(con, r['addr'])} [{r['state']}]")
        hooked.add(r["addr"])
    lines += ["", "[coverage]", f"enabled={1 if coverage else 0}", "",
              "[telemetry]", "; callsite=0x........  (a `call rel32` in the main loop that runs once per sim tick)", "",
              "[watch]", "; car0_speed=f32@[0x........]+0x..", ""]
    (rt / "gp4hook.ini").write_text("\n".join(lines))
    if coverage:
        starts = [r["addr"] for r in con.execute(
            "SELECT addr FROM functions WHERE kind='game' AND source IN ('entry','call','data-ptr','imm-ptr','sweep-call') "
            "AND size >= 1 ORDER BY addr") if r["addr"] not in hooked]
        (rt / "coverage_funcs.txt").write_text("\n".join(f"0x{a:08x}" for a in starts) + "\n")
    return {"hooks": len(hooked), "coverage": coverage}


def coverage_import(prefix: str) -> None:
    log = runtime_dir() / "coverage.log"
    con = dbm.connect()
    known = {r[0] for r in con.execute("SELECT addr FROM functions")}
    mark, seen = 0, {}
    for line in log.read_text().splitlines():
        m = re.match(r"--- mark (\d+)", line)
        if m:
            mark = int(m.group(1))
            continue
        h = re.match(r"0x([0-9a-f]{8})", line)
        if h and int(h.group(1), 16) in known:
            seen.setdefault(mark, set()).add(int(h.group(1), 16))
    with dbm.tx(con):
        for k, addrs in sorted(seen.items()):
            con.executemany("INSERT OR IGNORE INTO goals VALUES(?,?)", [(f"{prefix}-mark{k}", a) for a in addrs])
            print(f"goal {prefix}-mark{k}: {len(addrs)} functions first executed in this phase")


def main(a) -> None:
    if a.action == "prepare":
        print("runtime copy:", prepare(fresh=a.arg == "fresh"))
    elif a.action == "install":
        rt = prepare()
        dll = build_hook()
        shutil.copyfile(dll, rt / "dinput8.dll")
        res = write_ini(rt, only_runtime=a.arg == "runtime-only", coverage=os.environ.get("GP4RE_COVERAGE") == "1")
        print(f"installed {rt / 'dinput8.dll'}: {res['hooks']} hooks, coverage={'on' if res['coverage'] else 'off'}")
        print(f"launch the game from {rt} as usual; results land in gp4hook.log / coverage.log / telemetry.csv")
    elif a.action == "uninstall":
        p = runtime_dir() / "dinput8.dll"
        p.unlink(missing_ok=True)
        print("removed", p)
    elif a.action == "coverage-import":
        coverage_import(a.arg or "trace")
