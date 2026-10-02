"""Separate GP4's own code from linked libraries and compiler output.

Thief3-Decomp's measured lesson: library and compiler-generated shapes left in
the queue are where worker tokens die. This pass labels every function:

  game      - first-party GP4 code (the only thing workers are given)
  crt       - Visual C++ 6 runtime (sits at the end of .text)
  d3dx      - statically linked D3DX / DirectX-file code
  zlib      - zlib
  libpng    - libpng
  compiler  - thunks, EH unwind funclets
  lib?      - library-like but unconfirmed (stays queued, flagged for the lead)

Evidence: library-identifying strings, then propagation to callees reached only
from library code, then contiguity (linkers keep a library's objects together).
The lead confirms or overrides with ``gp4re work exclude/include``.
"""
from __future__ import annotations

import collections
import re

EVIDENCE = {
    "d3dx": re.compile(r"D3DX|ID3DX|d3dxof|DirectXFile|DisableD3DXPSGP|x file data|DebugSetMute|"
                       r"Bowtie|D3DXValid|Software\\Microsoft\\Direct3D", re.I),
    "zlib": re.compile(r"zlib|incorrect header check|invalid block type|invalid distance|"
                       r"oversubscribed|incomplete (literal|distance)|inflate |deflate ", re.I),
    "libpng": re.compile(r"libpng|png_|IHDR|IDAT|PLTE|tRNS|gAMA|Incompatible libpng|CRC error", re.I),
    "crt": re.compile(r"Runtime Error|R60\d\d|__MSVCRT|__GLOBAL_HEAP|<program name unknown>|"
                      r"Microsoft Visual C\+\+ Runtime|1#QNAN|1#SNAN|1#IND|1#INF|string too long|"
                      r"invalid string position|Unknown exception|GetLastActivePopup|bad allocation|"
                      r"floating point not loaded|_set_new_handler|SunMonTueWedThuFriSat|JanFebMar", re.I),
}
CRT_IMPORTS = {"kernel32.dll!LCMapStringA", "kernel32.dll!LCMapStringW", "kernel32.dll!GetStringTypeA",
               "kernel32.dll!GetStringTypeW", "kernel32.dll!GetLocaleInfoA", "kernel32.dll!GetCPInfo",
               "kernel32.dll!GetACP", "kernel32.dll!GetOEMCP", "kernel32.dll!HeapCreate",
               "kernel32.dll!HeapDestroy", "kernel32.dll!GetEnvironmentStrings",
               "kernel32.dll!FreeEnvironmentStringsA", "kernel32.dll!GetStartupInfoA",
               "kernel32.dll!GetCommandLineA", "kernel32.dll!GetVersion", "kernel32.dll!SetHandleCount",
               "kernel32.dll!GetStdHandle", "kernel32.dll!GetFileType", "kernel32.dll!UnhandledExceptionFilter",
               "kernel32.dll!RtlUnwind", "kernel32.dll!TlsAlloc", "kernel32.dll!TlsGetValue",
               "kernel32.dll!TlsSetValue", "kernel32.dll!SetUnhandledExceptionFilter",
               "kernel32.dll!IsBadReadPtr", "kernel32.dll!IsBadWritePtr", "kernel32.dll!IsBadCodePtr"}
LIBS = ("crt", "d3dx", "zlib", "libpng")

# Subsystem tags from a function's own strings - steer goals and claims (`--tag physics`).
TAG_WORDS = {
    "physics": r"tyre|tire|grip|slip|susp|aero|downforce|torque|brake|gear|engine|physic|skid|spin",
    "car": r"cars?|driver|cockpit|helmet|wheel|steer|pit",
    "ai": r"ai|computer car|ccline|opponent|overtak|racing line",
    "track": r"track|circuit|segment|kerb|verge|corner|\.dat",
    "render": r"d3d|texture|mesh|shader|vertex|render|draw|camera|light|lod|\.tex|\.gxm",
    "audio": r"sound|sfx|\.wav|\.bnk|speech",
    "ui": r"menu|button|font|screen|dialog|option|panel|hud|text_|pos_",
    "io": r"\.wad|\.ini|\.cfg|file|save|load",
    "replay": r"replay|telemetry|lap ?time|timing",
    "net": r"network|ip address|host|socket|multiplayer|session",
}
_TAGS = {k: re.compile(v, re.I) for k, v in TAG_WORDS.items()}


def tags_for(r: dict) -> str:
    texts = list(r["strings"].values())
    return ",".join(sorted(k for k, pat in _TAGS.items() if any(pat.search(s) for s in texts)))


def classify(rows: list[dict], entry: int) -> dict[int, tuple[str, str]]:
    """Return {addr: (kind, evidence)}."""
    by = {r["addr"]: r for r in rows}
    callers: dict[int, set[int]] = collections.defaultdict(set)
    for r in rows:
        for c in r["calls"] + r["tails"]:
            if c in by:
                callers[c].add(r["addr"])
    kind: dict[int, tuple[str, str]] = {}
    for r in rows:
        if r["style"] in ("thunk", "eh-funclet"):
            kind[r["addr"]] = ("compiler", r["style"])
            continue
        for lib, pat in EVIDENCE.items():
            hit = next((s for s in r["strings"].values() if pat.search(s)), None)
            if hit:
                kind[r["addr"]] = (lib, f"string {hit[:40]!r}")
                break
        else:
            imps = CRT_IMPORTS.intersection(r["imports"])
            if imps:
                kind[r["addr"]] = ("crt", f"import {sorted(imps)[0]}")
    kind.setdefault(entry, ("crt", "entry point (WinMainCRTStartup)"))

    # propagate: a function whose every caller is library X belongs to X
    changed = True
    while changed:
        changed = False
        for r in rows:
            a = r["addr"]
            if a in kind or not callers[a]:
                continue
            ks = {kind.get(c, ("game",))[0] for c in callers[a]}
            if len(ks) == 1 and (k := ks.pop()) in LIBS:
                kind[a] = (k, "only called from " + k)
                changed = True

    # contiguity: an unlabelled function between two same-library neighbours
    order = sorted(by)
    for i in range(1, len(order) - 1):
        a = order[i]
        if a in kind:
            continue
        prev_k = next((kind[b][0] for b in reversed(order[max(0, i - 3):i]) if b in kind and kind[b][0] in LIBS), None)
        next_k = next((kind[b][0] for b in order[i + 1:i + 4] if b in kind and kind[b][0] in LIBS), None)
        if prev_k and prev_k == next_k:
            kind[a] = (prev_k + "?", "between two " + prev_k + " functions")

    # the CRT tail: everything after the first long CRT run near the end of .text
    tail = [a for a in order if a >= _crt_tail_start(order, kind)]
    for a in tail:
        if a not in kind or kind[a][0].endswith("?"):
            kind[a] = ("crt", "CRT tail of .text")
    return {a: kind.get(a, ("game", "")) for a in order}


def _crt_tail_start(order: list[int], kind: dict) -> int:
    """Lowest address from which (scanning backwards from the end) the code is mostly CRT."""
    start = order[-1] + 1
    run_non = 0
    for a in reversed(order):
        k = kind.get(a, ("?",))[0]
        if k in ("crt", "compiler"):
            start, run_non = a, 0
        else:
            run_non += 1
            if run_non > 24:   # a long stretch of non-CRT code: the tail ended
                break
    return start
