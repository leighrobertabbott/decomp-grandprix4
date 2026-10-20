"""Lead helper: rerun deferred/blocked candidates after removing their blocker.

    python tools/lead_retry.py specs.json

specs.json maps addresses to a spec action for the newest build/scratch/<addr>/vN.cpp:
    "<full spec>"             replace the GP4_IMPL spec string
    "+globals=layout=<name>"  keep the spec, replace/append its globals option
    "="                       keep the file (rerun after a tooling, layout or exemption change)
Each address is released/requeued, the candidate (the accepted src/ file when the
function is already accepted - a strengthening rerun - else the newest scratch file)
is copied to v(N+1), tried and, on PASS, accepted. The verifier still decides; this only saves the lead retyping.
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ENV = dict(os.environ, GP4RE_AGENT="lead")
IMPL = re.compile(r'(GP4_IMPL\([^,]+,[^,]+,\s*)"([^"]*)"')


def gp4re(*args):
    r = subprocess.run([sys.executable, "-m", "gp4re", *args], capture_output=True, text=True,
                       env=ENV, cwd=ROOT, timeout=4000)
    return r.stdout + r.stderr


def retry(addr: str, action: str) -> str:
    d = ROOT / "build" / "scratch" / f"{int(addr, 16):08x}"
    d.mkdir(parents=True, exist_ok=True)
    files = sorted(d.glob("v*.cpp"), key=lambda p: int(re.search(r"v(\d+)\.cpp$", p.name).group(1)))
    n = (int(re.search(r"v(\d+)\.cpp$", files[-1].name).group(1)) + 1) if files else 1
    accepted = sorted(ROOT.glob(f"src/gp4/*/*/{int(addr, 16):08x}.cpp"))
    if accepted:          # strengthening an accepted function: start from the accepted source
        last = accepted[0]
    elif files:
        last = files[-1]
    else:
        return "no candidate file"
    src = re.sub(r"\n// \(lead rerun after a tooling outage\)\n?", "\n", last.read_text())
    m = IMPL.search(src)
    if not m:
        return f"no GP4_IMPL in {last.name}"
    spec = m.group(2)
    if action == "=":
        pass   # unchanged source, re-verified with `work try --rerun` (the blocker was outside the file)
    elif action.startswith("+globals="):
        spec = re.sub(r"\s*;\s*globals=[^;\s]+", "", spec) + "; " + action[1:]
    else:
        spec = action
    src = IMPL.sub(lambda k: k.group(1) + '"' + spec + '"', src, count=1)
    out = d / f"v{n}.cpp"
    out.write_text(src)
    # Claim it as the lead (todo/deferred/expired) so no worker picks it up mid-retry;
    # an accepted function (strengthening rerun) is simply re-verified and re-accepted.
    gp4re("work", "claim", "--addr", addr)
    t = gp4re("work", "try", addr, str(out.relative_to(ROOT)), "--cap", "99", "--rerun")
    if "VERDICT: PASS" not in t:
        lines = [l.strip()[:220] for l in t.splitlines()
                 if l.startswith(("VERDICT", "emu", "BUILD", "REFUSED", "shape")) or "warning" in l]
        return f"{out.name} NOT PASS: " + " | ".join(lines)
    a = gp4re("work", "accept", addr, str(out.relative_to(ROOT)))
    warn = [l.strip()[:160] for l in t.splitlines() if "warning: unvaried" in l]
    if "ACCEPTED" not in a:
        return "ACCEPT FAILED: " + a[-300:]
    # accepted-runtime (emu skipped or every case external) is NOT a T1 pass: say so
    tag = "ACCEPTED-RUNTIME (pending in-game check)" if "accepted-runtime" in a else "ACCEPTED"
    return tag + (f"  ({warn[0]})" if warn else "")


if __name__ == "__main__":
    for addr, action in json.loads(Path(sys.argv[1]).read_text()).items():
        print(addr, retry(addr, action), flush=True)
