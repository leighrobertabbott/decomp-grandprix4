"""Structural faithfulness audit (verification tier T0.5, Burnout-style 'asmaudit').

Compares what the ORIGINAL function touches (callees, Win32 imports, globals,
strings) with what the compiled RECONSTRUCTION touches. Catches stubbed-out or
hallucinated bodies even when no input case exercises the difference.

Tiers: A = same shape, B = close, C = diverges (gate fails), T = trivial.
"""
from __future__ import annotations

import re

from .analyze import Analyzer, _MEM, _int, parse_mem, split_operands
from .binary import Binary
from .emu import Impl


def recon_facts(dll: Binary, gp4: Binary, rec: Impl, registry: list[Impl]) -> dict:
    by_impl = {r.impl: r.addr for r in registry}
    a = Analyzer(dll, log=lambda *x: None)
    a.starts = set(by_impl)
    gp4_text = gp4.section(".text")
    callees, imports, globals_, strings, seen = set(), set(), set(), set(), set()
    x87 = n_cond = 0
    todo = [rec.impl]
    while todo:
        s = todo.pop()
        if s in seen:
            continue
        seen.add(s)
        f = a.explore(s)
        for c in f.calls | f.tails:
            if c in by_impl and c != rec.impl:
                callees.add(by_impl[c])
            elif gp4_text.contains(c):
                callees.add(c)          # direct calls/tails to fixed original addresses
            elif dll.in_image(c):
                todo.append(c)          # private helper inside the DLL: audit it too
        for ins in f.insns.values():
            x87 += ins.mn.startswith("f")
            n_cond += ins.mn.startswith("j") and ins.mn != "jmp"
            prev_imm = None
            for o in split_operands(ins.ops):
                m = _MEM.search(o)
                vals = []
                if m:
                    base, index, _, disp = parse_mem(m.group(1))
                    if disp in gp4.imports and base is None and index is None:
                        imports.add(gp4.imports[disp])
                        continue
                    vals.append(disp)
                else:
                    v = _int(o)
                    if v is not None:
                        vals.append(v)
                for v in vals:
                    if gp4_text.contains(v):
                        callees.add(v)
                    elif gp4.in_image(v) and v > gp4.base + 0x1000:
                        globals_.add(v)
                    elif dll.in_image(v) and (txt := dll.cstring(v)):
                        strings.add(txt)
    return {"callees": callees, "imports": imports, "globals": globals_, "strings": strings,
            "x87": x87, "n_cond": n_cond}


def audit(con, gp4: Binary, dll: Binary, rec: Impl, registry: list[Impl]) -> dict:
    addr = rec.addr
    f = con.execute("SELECT * FROM functions WHERE addr=?", (addr,)).fetchone()
    if not f:
        return {"gate": "shape", "status": "fail", "reason": f"{addr:#x} is not a known function start"}
    o_callees = {r[0] for r in con.execute("SELECT dst FROM edges WHERE src=?", (addr,))}
    o_imports = {r[0] for r in con.execute("SELECT import FROM func_imports WHERE addr=?", (addr,))}
    o_globals = {r[0] for r in con.execute("SELECT gaddr FROM func_globals WHERE addr=?", (addr,))}
    o_strings = {r[0]: r[1] for r in con.execute(
        "SELECT g.gaddr, s.text FROM func_globals g JOIN strings s ON s.gaddr=g.gaddr WHERE g.addr=?", (addr,))}
    r = recon_facts(dll, gp4, rec, registry)
    # a string global is satisfied either by referencing GP4's copy or an identical literal
    o_globals_nostr = {g for g in o_globals if g not in o_strings or o_strings[g] not in r["strings"]}
    missing_callees = sorted(o_callees - r["callees"])
    extra_callees = sorted(r["callees"] - o_callees)
    missing_imports = sorted(o_imports - r["imports"])
    extra_imports = sorted(r["imports"] - o_imports)
    # compilers fold "base + field offset" differently: an address within FOLD bytes
    # above the other side's address counts as the same object
    FOLD = 0x800

    def near(x, ys):
        return any(0 <= x - y < FOLD or 0 <= y - x < FOLD for y in ys)
    o_unmatched = {g for g in o_globals_nostr if not near(g, r["globals"])}
    r_unmatched = {g for g in r["globals"] if not near(g, o_globals_nostr)}
    matched = len(o_globals_nostr) - len(o_unmatched)
    denom = matched + len(o_unmatched) + len(r_unmatched)
    jac = matched / denom if denom else 1.0
    if missing_callees or extra_callees or missing_imports or extra_imports:
        tier = "C"                 # every call target is part of the contract
    elif f["n_insn"] <= 3 and jac >= 0.9:
        tier = "T"
    elif jac >= 0.9:
        tier = "A"
    elif jac >= 0.6:
        tier = "B"
    else:
        tier = "C"
    return {
        "gate": "shape", "status": "fail" if tier == "C" else "pass", "tier": tier,
        "missing_callees": [hex(x) for x in missing_callees], "extra_callees": [hex(x) for x in extra_callees],
        "missing_imports": missing_imports, "extra_imports": extra_imports,
        "globals_jaccard": round(jac, 3),
        "missing_globals": [hex(x) for x in sorted(o_unmatched)][:20],
        "extra_globals": [hex(x) for x in sorted(r_unmatched)][:20],
        "x87": {"original": f["x87"], "reconstruction": r["x87"]},
        "branches": {"original": f["n_cond"], "reconstruction": r["n_cond"]},
    }
