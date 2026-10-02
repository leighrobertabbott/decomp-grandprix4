"""Gate runner behind ``gp4re work try/accept`` and ``gp4re verify``.

Gates, in order (all must pass):
  lint   - one ``// FUNCTION: GP4 0x...`` + one GP4_IMPL for the target; banned constructs
  build  - accepted reconstructions + runtime + the candidate compile and link (MSVC x86, x87)
  shape  - structural audit vs the original (tier A/B/T pass, C fails)
  emu    - differential emulation vs the original (bit-exact unless the spec sets tol=)

``emu=skip:<reason>`` (e.g. the function calls Win32) or an all-external run
downgrades a shape-A result to ``pass-pending-runtime``: accepted, but it must be
confirmed in-game through the hook DLL later (T2).
"""
from __future__ import annotations

import hashlib
import os
import re
from pathlib import Path

from . import db as dbm
from . import emu, msvc, shape
from .analyze import Analyzer
from .binary import Binary
from .paths import REPO, build_dir, find_exe
from .spec import SpecError, parse

_GP4 = None
BANNED = [
    (re.compile(r"^\s*#\s*include\s*<(?!stdint\.h|stddef\.h|string\.h|gp4/)[^>]+>", re.M),
     "system headers other than <stdint.h>/<stddef.h>/<string.h> (no <math.h>: use gp4::x87 for bit-exact math)"),
    (re.compile(r"\b__asm\b|\b_emit\b|\bnaked\b"), "inline asm (use gp4::x87 helpers or gp4::call_regs)"),
    (re.compile(r"#\s*pragma\s+(optimize|inline_depth|auto_inline|function|intrinsic)"), "codegen pragmas"),
    (re.compile(r"\bvolatile\b"), "volatile (use gp4::x87::to_f32/to_f64 to force a store's rounding)"),
]


def gp4_binary() -> Binary:
    global _GP4
    if _GP4 is None:
        _GP4 = Binary.load(find_exe())
    return _GP4


def impl_pattern(addr: int) -> re.Pattern:
    return re.compile(rf"GP4_IMPL\(\s*0x0*{addr:x}\b", re.I)


def lint_candidate(addr: int, cand: Path) -> list[str]:
    txt = cand.read_text(errors="replace")
    probs = []
    impls = re.findall(r"GP4_IMPL\(\s*(0x[0-9a-fA-F]+)", txt)
    if [int(x, 16) for x in impls] != [addr]:
        probs.append(f"exactly one GP4_IMPL, for 0x{addr:08x}, is required (found {impls or 'none'})")
    if not re.search(rf"//\s*FUNCTION:\s*GP4\s+0x0*{addr:x}\b", txt, re.I):
        probs.append(f"missing '// FUNCTION: GP4 0x{addr:08x}' line above the definition")
    code = re.sub(r"//[^\n]*|/\*.*?\*/", "", txt, flags=re.S)
    for pat, why in BANNED:
        if pat.search(code):
            probs.append(f"not allowed: {why}")
    return probs


def accepted_sources(root: Path, exclude_addr: int | None) -> list[Path]:
    pat = impl_pattern(exclude_addr) if exclude_addr is not None else None
    out = []
    for p in msvc.sources(root):
        if pat and pat.search(p.read_text(errors="replace")):
            continue
        out.append(p)
    return out


def code_sha(dll: Binary, impl: int) -> str:
    a = Analyzer(dll, log=lambda *x: None)
    f = a.explore(impl)
    h = hashlib.sha1()
    for va in sorted(f.insns):
        h.update(dll.read(va, f.insns[va].size))
    return h.hexdigest()[:16]


def skip_problems(con, addr: int, reason: str | None) -> list[str]:
    """A runtime-only exception needs an evidenced external API dependency."""
    if reason is None:
        return []
    match = re.fullmatch(r"win32\s+([A-Za-z_][A-Za-z0-9_@?$]*|#\d+)", reason)
    if not match:
        return ["emu skip requires 'win32 <Api>' and an evidenced import dependency"]
    api = match.group(1).lower()
    seen, pending = set(), [addr]
    while pending:
        current = pending.pop()
        if current in seen:
            continue
        seen.add(current)
        imports = [r[0] for r in con.execute("SELECT import FROM func_imports WHERE addr=?", (current,))]
        if any(name.rsplit("!", 1)[-1].lower() == api for name in imports):
            return []
        pending.extend(r[0] for r in con.execute("SELECT dst FROM edges WHERE src=?", (current,)))
    return [f"emu skip names {match.group(1)}, but the original's call graph has no matching import"]


def run(addr: int, candidate: Path | None = None, root: Path = REPO, agent: str = "lead",
        cases: int | None = None) -> dict:
    con = dbm.connect()
    from .config import oracle
    fpucw = dbm.meta_get(con, "fpucw") or oracle("fpu_cw")   # explicit override, else config
    gates: list[dict] = []
    if candidate is not None:
        probs = lint_candidate(addr, candidate)
        files = accepted_sources(root, addr) + [candidate]
        src_file = msvc._rel(candidate, root)
    else:
        files = msvc.sources(root)
        hits = [p for p in files if impl_pattern(addr).search(p.read_text(errors="replace"))]
        probs = [] if len(hits) == 1 else [f"expected one source with GP4_IMPL(0x{addr:08x}), found {len(hits)}"]
        if len(hits) == 1:
            probs += lint_candidate(addr, hits[0])
        src_file = msvc._rel(hits[0], root) if hits else None
    gates.append({"gate": "lint", "status": "fail" if probs else "pass", "problems": probs})
    if probs:
        return _summary(addr, gates)
    out_dir = build_dir() / "try" / re.sub(r"[^\w.-]", "_", agent) / f"{addr:08x}.{os.getpid()}"
    b = msvc.build_recon(files=files, root=root, out_dir=out_dir)
    gates.append(b)
    if b["status"] != "pass":
        return _summary(addr, gates)
    dll = Binary.load(b["dll"])
    registry = emu.read_registry(dll)
    rec = next((r for r in registry if r.addr == addr), None)
    if rec is None:
        gates.append({"gate": "registry", "status": "fail",
                      "problems": ["GP4_IMPL record not found in the built DLL"]})
        return _summary(addr, gates)
    try:
        spec = parse(rec.spec)
    except SpecError as e:
        gates.append({"gate": "spec", "status": "fail", "problems": [f"spec {rec.spec!r}: {e}"]})
        return _summary(addr, gates)
    gp4 = gp4_binary()
    probs = skip_problems(con, addr, spec.emu_skip)
    if probs:
        gates.append({"gate": "spec", "status": "fail", "problems": probs})
        return _summary(addr, gates)
    gates.append(shape.audit(con, gp4, dll, rec, registry))
    globals_info = [r[0] for r in con.execute("SELECT gaddr FROM func_globals WHERE addr=?", (addr,))]
    gates.append(emu.differential(
        gp4, dll, rec, spec, fpucw=fpucw, globals_info=globals_info,
        cases=cases if cases is not None else max(spec.cases, oracle("cases")),
        function_starts={r[0] for r in con.execute("SELECT addr FROM functions")},
    ))
    out = _summary(addr, gates)
    out.update(name=rec.name, spec=rec.spec, source_file=src_file, code_sha=code_sha(dll, rec.impl),
               field_macros=len(re.findall(r"GP4_FIELD\(", (candidate or (root / src_file)).read_text())))
    return out


def _summary(addr: int, gates: list[dict]) -> dict:
    by = {g["gate"]: g for g in gates}
    failed = [g["gate"] for g in gates if g["status"] == "fail"]
    if failed:
        overall = "fail"
    else:
        e, sh = by.get("emu", {}), by.get("shape", {})
        if e.get("status") == "pass":
            overall = "pass"
        elif e.get("status") in ("skipped", "inconclusive") and sh.get("tier") in ("A", "T") \
                and (e.get("status") == "skipped" or _all_external(e)):
            overall = "pass-pending-runtime"
        else:
            overall = "fail"
            failed.append("emu-inconclusive")
    return {"addr": f"0x{addr:08x}", "overall": overall, "failed": failed, "gates": gates}


def _all_external(e: dict) -> bool:
    reasons = e.get("invalid_reasons", {})
    return bool(reasons) and all(k.startswith("external") for k in reasons)
