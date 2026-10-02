"""Settings from config/project.toml, with defaults for anything missing."""
from __future__ import annotations

import copy
import tomllib
from functools import lru_cache

from .paths import REPO

DEFAULTS = {
    "game": {"exe": "Grand-Prix-4/Grand Prix 4/GP4.exe", "sha256": ""},
    "oracle": {"fpu_cw": 0x037F, "init_calls": [], "cases": 200, "max_cases": 1500, "time_budget_s": 25,
               "max_insns": 20_000_000, "min_block_coverage": 0.90, "min_branch_coverage": 0.70,
               "ni_cases": 6, "lazy_pages": 256, "compare_cases": 120},
    "queue": {"lease_hours": 2, "default_cap": 8},
    "compiler": {"msvc6": {"dir": "toolchain/msvc6"}},
}


def _merge(base: dict, over: dict) -> dict:
    out = copy.deepcopy(base)
    for k, v in over.items():
        out[k] = _merge(out[k], v) if isinstance(v, dict) and isinstance(out.get(k), dict) else v
    return out


@lru_cache(maxsize=None)
def cfg() -> dict:
    p = REPO / "config" / "project.toml"
    if not p.exists():
        return copy.deepcopy(DEFAULTS)
    with open(p, "rb") as fh:
        return _merge(DEFAULTS, tomllib.load(fh))


def oracle(key: str):
    return cfg()["oracle"][key]
