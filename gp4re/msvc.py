"""MSVC x86 toolchain driver.

Reconstructions are compiled with a modern MSVC targeting x86 + x87
(``/arch:IA32``) because GP4's float code is x87. Output is a no-CRT, fixed-base
DLL (``build/recon.dll``) that both the emulator oracle and (with a different
link step) the in-game hook DLL consume.

If the user supplies Visual C++ 6.0 (``GP4RE_VC6_DIR``), a matching-decomp
track becomes possible; see docs/STRATEGY.md.
"""
from __future__ import annotations

import hashlib
import json
import os
import shutil
import subprocess
import uuid
from pathlib import Path

from .paths import REPO, build_dir, state_dir

VSWHERE = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe")
DLL_BASE = 0x10000000
CFLAGS = ["/nologo", "/c", "/O2", "/Oy-", "/arch:IA32", "/fp:precise", "/GS-", "/Gy-", "/GR-",
          "/EHs-c-", "/Zl", "/W3", "/std:c++17", "/Zc:threadSafeInit-", "/D_CRT_SECURE_NO_WARNINGS",
          "/DGP4_RECON=1"]
LFLAGS = ["/nologo", "/DLL", "/NOENTRY", "/NODEFAULTLIB", "/FIXED", f"/BASE:{DLL_BASE:#x}",
          "/OPT:NOREF", "/OPT:NOICF", "/SAFESEH:NO"]


class ToolchainError(RuntimeError):
    pass


def _vcvars() -> Path:
    env = os.environ.get("GP4RE_VCVARS")
    if env:
        return Path(env)
    if VSWHERE.exists():
        out = subprocess.run([str(VSWHERE), "-latest", "-products", "*", "-requires",
                              "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property",
                              "installationPath"], capture_output=True, text=True).stdout.strip()
        for line in out.splitlines():
            p = Path(line) / "VC" / "Auxiliary" / "Build" / "vcvars32.bat"
            if p.exists():
                return p
    raise ToolchainError("MSVC x86 toolchain not found (install VS Build Tools with C++ x86/x64, "
                         "or set GP4RE_VCVARS to vcvars32.bat)")


def msvc_env() -> dict[str, str]:
    cache = state_dir() / "msvc_env.json"
    if cache.exists():
        return json.loads(cache.read_text())
    bat = _vcvars()
    out = subprocess.run(f'cmd /s /c ""{bat}" >nul && set"', capture_output=True, text=True, shell=True)
    env = {}
    for line in out.stdout.splitlines():
        k, sep, v = line.partition("=")
        if sep:
            env[k] = v
    if "INCLUDE" not in env:
        raise ToolchainError(f"vcvars32 failed: {out.stderr[:400]}")
    cache.write_text(json.dumps(env))
    return env


def _run(cmd: list[str], env: dict, cwd: Path) -> tuple[int, str]:
    # CreateProcess resolves the exe with the *parent's* PATH, so resolve it ourselves.
    path = env.get("PATH") or env.get("Path") or ""
    exe = shutil.which(cmd[0], path=path)
    if not exe:
        raise ToolchainError(f"{cmd[0]} not found on the vcvars32 PATH")
    p = subprocess.run([exe, *cmd[1:]], env=env, cwd=cwd, capture_output=True, text=True, errors="replace")
    return p.returncode, (p.stdout + p.stderr).strip()


def sources(root: Path = REPO) -> list[Path]:
    src = sorted((root / "src").rglob("*.cpp")) + sorted((root / "src").rglob("*.c"))
    return [p for p in src if "hook" not in p.parts]


def _publish_object(tmp: Path, obj: Path) -> None:
    """Atomically create a content-keyed cache entry without replacing readers.

    A concurrent linker may have the winning object open on Windows. Hard-link
    publication creates the name only if absent, avoiding replace-on-open errors.
    Both paths are in the object cache directory, hence on the same volume.
    """
    try:
        os.link(tmp, obj)
    except FileExistsError:
        pass  # Another compiler published this content key first.
    finally:
        tmp.unlink(missing_ok=True)


def build_recon(files: list[Path] | None = None, root: Path = REPO, out_dir: Path | None = None,
                out_name: str = "recon") -> dict:
    """Compile reconstructions + runtime into <out_dir>/<out_name>.dll.

    Objects are cached by content hash in build/obj (shared by every worker, written
    atomically), so a try only recompiles the candidate and relinks.
    """
    env = msvc_env()
    bdir = out_dir or (build_dir() / out_name)
    bdir.mkdir(parents=True, exist_ok=True)
    obj_dir = build_dir() / "obj"
    obj_dir.mkdir(parents=True, exist_ok=True)
    inc = root / "include"
    hdr_hash = hashlib.sha1(b"".join(p.read_bytes() for p in sorted(inc.rglob("*.h")))).hexdigest()[:10]
    files = files if files is not None else sources(root)
    objs, log = [], []
    for src in files:
        key = hashlib.sha1(src.read_bytes() + hdr_hash.encode() + " ".join(CFLAGS).encode()).hexdigest()[:16]
        obj = obj_dir / f"{src.stem}.{key}.obj"
        if not obj.exists():
            tmp = obj_dir / f"{src.stem}.{key}.{uuid.uuid4().hex}.tmp.obj"
            rc, out = _run(["cl", *CFLAGS, f"/I{inc}", f"/Fo{tmp}", str(src)], env, root)
            if rc != 0:
                tmp.unlink(missing_ok=True)
                return {"gate": "build", "status": "fail", "file": _rel(src, root), "log": _trim(out)}
            _publish_object(tmp, obj)
            if "warning" in out:
                log.append(_trim(out, 1500))
        objs.append(obj)
    dll = bdir / f"{out_name}.dll"
    rc, out = _run(["link", *LFLAGS, f"/OUT:{dll}", f"/MAP:{bdir / (out_name + '.map')}", *map(str, objs)], env, root)
    if rc != 0:
        return {"gate": "build", "status": "fail", "file": "<link>", "log": _trim(out)}
    return {"gate": "build", "status": "pass", "dll": str(dll), "objects": len(objs),
            "warnings": log[:5]}


def _rel(p: Path, root: Path) -> str:
    try:
        return p.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return str(p)


def _trim(s: str, n: int = 4000) -> str:
    return s if len(s) <= n else s[:n] + "\n...[truncated]"
