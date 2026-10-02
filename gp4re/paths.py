"""Path resolution.

Every git worktree has its own source tree (``src/``), but analysis output and the
work ledger are shared and live in the *main* checkout. ``main_root()`` finds that
checkout through git's common dir so N worker worktrees all talk to one ledger.
"""
from __future__ import annotations

import os
import subprocess
from functools import lru_cache
from pathlib import Path

PKG = Path(__file__).resolve().parent
REPO = PKG.parent  # the checkout (or worktree) this copy of gp4re lives in


@lru_cache(maxsize=None)
def main_root() -> Path:
    env = os.environ.get("GP4RE_HOME")
    if env:
        return Path(env).resolve()
    try:
        out = subprocess.run(
            ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"],
            cwd=REPO, capture_output=True, text=True, check=True,
        ).stdout.strip()
        common = Path(out)
        if common.name == ".git":
            return common.parent.resolve()
    except (OSError, subprocess.CalledProcessError):
        pass
    return REPO


def state_dir() -> Path:
    d = main_root() / "state"
    d.mkdir(parents=True, exist_ok=True)
    return d


def analysis_dir() -> Path:
    d = main_root() / "analysis"
    d.mkdir(parents=True, exist_ok=True)
    return d


def db_path() -> Path:
    return state_dir() / "gp4re.sqlite"


def progress_dir() -> Path:
    # committed, per-branch: lives in the current worktree
    d = REPO / "progress"
    d.mkdir(parents=True, exist_ok=True)
    return d


def build_dir() -> Path:
    d = REPO / "build"
    d.mkdir(parents=True, exist_ok=True)
    return d


INPUT_DIRS = ("Grand-Prix-4", "input")


@lru_cache(maxsize=None)
def find_game_dir() -> Path | None:
    """Locate the user's GP4 install (the folder that contains GP4.exe)."""
    env = os.environ.get("GP4RE_GAME_DIR")
    if env and (Path(env) / "GP4.exe").exists():
        return Path(env)
    from .config import cfg
    configured = main_root() / cfg()["game"]["exe"]
    if configured.exists():
        return configured.parent
    root = main_root()
    for d in INPUT_DIRS:
        base = root / d
        if not base.exists():
            continue
        for p in sorted(base.rglob("*")):
            if p.is_file() and p.name.lower() == "gp4.exe":
                return p.parent
    return None


def find_exe() -> Path:
    env = os.environ.get("GP4RE_EXE")
    if env:
        return Path(env)
    g = find_game_dir()
    if not g:
        raise SystemExit(
            "GP4.exe not found. Put your Grand Prix 4 install in ./Grand-Prix-4/ or ./input/ "
            "(or set GP4RE_EXE)."
        )
    return g / "GP4.exe"
