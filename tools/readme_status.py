"""Regenerate the README progress block from the shared ledger.

    python tools/readme_status.py [--readme README.md]

Reads progress/report.json (the numbers `python -m gp4re status` prints) and state/gp4re.sqlite,
and replaces the text between `<!-- status:begin -->` and `<!-- status:end -->` with progress
badges and a per-region table. Run it after `python -m gp4re export` at every checkpoint so the
README always matches the ledger (progress/ledger.txt, progress/report.json).
"""
import argparse
import json
import sqlite3
import time
from pathlib import Path
from urllib.parse import quote

ROOT = Path(__file__).resolve().parent.parent
BEGIN, END = "<!-- status:begin -->", "<!-- status:end -->"
REGIONS = {   # label -> (what it is, address range); ranges from progress/regions.json
    "R1-asm-core": "Simulation core (hand-written x87 assembly)",
    "R2-cpp-renderer": "Renderer and front end (Direct3D 8 C++)",
    "R3-c-legacy": "Game logic (GP3-heritage C and assembly)",
    "R4-libs": "Libraries: D3DX, libpng, zlib (excluded)",
    "R5-cpp-mixed": "Mixed C++ and DirectX error tables",
    "R6-crt": "Visual C++ 6 runtime",
}


def badge(label: str, value: str, colour: str) -> str:
    esc = lambda s: quote(s.replace("-", "--").replace("_", "__").replace(" ", "_"), safe="_.")
    return f"![{label}](https://img.shields.io/badge/{esc(label)}-{esc(value)}-{colour})"


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--readme", default=str(ROOT / "README.md"))
    readme = Path(ap.parse_args().readme)
    report = json.loads((ROOT / "progress" / "report.json").read_text())
    states = report["by_state"]
    con = sqlite3.connect(ROOT / "state" / "gp4re.sqlite")
    rows = con.execute("SELECT f.region, l.state, count(*), SUM(f.size) FROM ledger l JOIN functions f USING(addr) "
                       "GROUP BY f.region, l.state").fetchall()
    ranges = {r["label"]: (r["start"], r["end"]) for r in json.loads((ROOT / "progress" / "regions.json").read_text())["regions"]}
    regions: dict[str, dict[str, list[int]]] = {}
    for region, state, n, size in rows:
        regions.setdefault(region or "?", {}).setdefault(state, [0, 0])
        regions[region or "?"][state][0] += n
        regions[region or "?"][state][1] += size or 0
    verified, pending = states.get("accepted", 0), states.get("accepted-runtime", 0)
    lines = [BEGIN, "",
             " ".join([badge("code", f"{report['progress_pct']:.2f}%", "blue"),
                       badge("verified", f"{verified:,} functions", "brightgreen"),
                       badge("pending in-game", f"{pending:,}", "yellow"),
                       badge("deferred", f"{states.get('deferred', 0):,}", "orange"),
                       badge("todo", f"{states.get('todo', 0):,}", "lightgrey")]),
             "",
             f"**{report['progress_pct']:.2f}%** of GP4.exe's game code ({report['accepted_bytes']:,} of "
             f"{report['game_bytes']:,} bytes) is reconstructed and accepted: **{verified:,}** functions verified by "
             f"differential emulation, **{pending:,}** accepted pending the in-game check, "
             f"{states.get('deferred', 0):,} deferred with a recorded blocker, {states.get('todo', 0):,} still to do.",
             "",
             "| Region | Address range | Verified | Pending in-game | Deferred | To do | Code accepted |",
             "|---|---|---:|---:|---:|---:|---:|"]
    for label in sorted(regions):
        d = regions[label]
        total = sum(v[1] for k, v in d.items() if k != "excluded")
        done = d.get("accepted", [0, 0])[1] + d.get("accepted-runtime", [0, 0])[1]
        share = f"{100 * done / total:.1f}%" if total else "excluded"
        lo, hi = ranges.get(label, ("?", "?"))
        lines.append(f"| {REGIONS.get(label, label)} | `{lo}`-`{hi}` | {d.get('accepted', [0])[0]:,} | "
                     f"{d.get('accepted-runtime', [0])[0]:,} | {d.get('deferred', [0])[0]:,} | "
                     f"{d.get('todo', [0])[0]:,} | {share} |")
    lines += ["",
              f"<sub>Generated {time.strftime('%Y-%m-%d')} from the ledger by `python tools/readme_status.py`; "
              "per-function state is in [progress/ledger.txt](progress/ledger.txt).</sub>",
              "", END]
    text = readme.read_text(encoding="utf-8")
    start, end = text.find(BEGIN), text.find(END)
    if start < 0 or end < 0:
        raise SystemExit(f"{readme} has no status markers")
    readme.write_text(text[:start] + "\n".join(lines) + text[end + len(END):], encoding="utf-8")
    print(f"{readme.name} progress block updated: {report['progress_pct']:.2f}% code, "
          f"{verified} verified, {pending} pending in-game")


if __name__ == "__main__":
    main()
