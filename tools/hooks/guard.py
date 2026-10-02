#!/usr/bin/env python3
"""PreToolUse guard for gp4-worker sub-agents (wired in .claude/agents/gp4-worker.md).

Workers may write only build/scratch/. Everything else - src/, include/, progress/,
prompts/, the tools, .claude/, the game folders - belongs to the lead (the gp4re
tools themselves write src/ on accept; the guard only constrains the agent's own
Write/Edit calls and shell commands). Shell commands that change git state, delete
recursively, run inline interpreters, use lead-only gp4re commands or write
outside build/scratch/ are refused.

Claude Code passes the call as JSON on stdin; exit 2 + a reason on stderr blocks
it. Any error inside the guard also blocks (fail closed).
"""
from __future__ import annotations

import json
import os
import re
import shlex
import sys
from pathlib import Path

SCRATCH = "build/scratch/"
LEAD_ONLY = ("analyze", "exclude", "include", "requeue", "sweep", "names", "goal", "export", "fpucw",
             "runtime", "ghidra")
DENY_BASH = [
    (r"\bgit\b(?!\s+(status|diff|log|show)\b)", "git is the lead's (read-only status/diff/log/show are allowed)"),
    (r"\brm\s+(-\w*[rR]\w*|--recursive)\b|\bRemove-Item\b.*-Recurse|\brmdir\s+/s\b", "recursive delete"),
    (r"\b(python[\d.]*|py|node|perl|ruby|bash|sh|pwsh|powershell|cmd)(\.exe)?\s+(-c\b|-e\b|-\s|-$|/c\b|<<|<\s)",
     "inline scripts - run gp4re commands instead"),
    (r"--force\b", "--force is the lead's"),
    (r"\bGP4RE_(HOME|EXE|GAME_DIR|VCVARS)\s*=", "the GP4RE_* paths are fixed"),
    (r"Grand-Prix-4|runtime[\\/]GP4", "the game folders are off limits"),
    (r"\b(curl|wget|Invoke-WebRequest|iwr)\b", "network access"),
]
WRITERS = {"tee", "touch", "truncate", "rm", "rmdir", "cp", "mv", "ln", "install", "mkdir", "chmod", "del",
           "copy", "move", "xcopy", "robocopy", "set-content", "out-file", "new-item", "rename-item"}


def deny(reason: str) -> None:
    sys.stderr.write(f"Blocked by the gp4 worker guard: {reason}\n")
    sys.exit(2)


def check_path(raw: str, cwd: Path, project: Path) -> str | None:
    if raw.lower() in ("/dev/null", "nul", "/dev/stdout", "/dev/stderr"):
        return None
    p = Path(os.path.expanduser(raw.replace("\\", "/")))
    p = (p if p.is_absolute() else cwd / p).resolve()
    try:
        rel = p.relative_to(project).as_posix()
    except ValueError:
        return f"{raw} is outside the repository"
    if (rel + "/").startswith(SCRATCH):
        return None
    return f"{raw}: workers write only under {SCRATCH} (src/ is written by `work accept`)"


def bash_targets(cmd: str) -> list[str]:
    try:
        lex = shlex.shlex(cmd, posix=True, punctuation_chars=";&|<>")
        lex.whitespace_split = True
        toks = list(lex)
    except ValueError:
        return ["<unparsable command>"]
    out, seg = [], []
    for t in toks + [";"]:
        if t and set(t) <= set(";&|"):
            out += _seg_targets(seg)
            seg = []
        else:
            seg.append(t)
    return out


def _seg_targets(seg: list[str]) -> list[str]:
    out, words, i = [], [], 0
    while i < len(seg):
        t = seg[i]
        if t.isdigit() and i + 1 < len(seg) and seg[i + 1][:1] in "<>":
            i += 1
            continue
        if t in (">&", "<&"):
            i += 2
            continue
        if t in (">", ">>", ">|", "&>"):
            if i + 1 < len(seg) and not seg[i + 1].startswith("&"):
                out.append(seg[i + 1])
            i += 2
            continue
        if t in ("<", "<<", "<<<"):
            i += 2
            continue
        words.append(t)
        i += 1
    while words and re.fullmatch(r"\w+=.*", words[0]):
        words.pop(0)
    if not words:
        return out
    c = os.path.basename(words[0]).lower().replace(".exe", "")
    args = [w for w in words[1:] if not w.startswith("-")]
    if c in WRITERS:
        out += args[-1:] if c in ("cp", "mv", "ln", "install", "copy", "move", "xcopy", "robocopy") else args
    elif c == "sed" and any(w.startswith("-i") for w in words[1:]):
        out += args[1:]
    return out


def main() -> None:
    ev = json.load(sys.stdin)
    tool = ev.get("tool_name", "")
    data = ev.get("tool_input") or {}
    cwd = Path(ev.get("cwd") or os.getcwd()).resolve()
    project = Path(os.environ.get("CLAUDE_PROJECT_DIR") or cwd).resolve()
    if tool in ("Write", "Edit", "MultiEdit", "NotebookEdit"):
        why = check_path(data.get("file_path") or data.get("notebook_path") or "", cwd, project)
        if why:
            deny(why)
        return
    if tool == "Bash":
        cmd = data.get("command", "")
        for pat, why in DENY_BASH:
            if re.search(pat, cmd, re.I):
                deny(why)
        m = re.findall(r"\bgp4re\s+(?:--agent\s+\S+\s+)?(\w+)(?:\s+(\w+))?", cmd)
        for top, sub in m:
            if top in LEAD_ONLY or (top == "work" and sub in LEAD_ONLY):
                deny(f"`gp4re {top} {sub}`".strip() + " is the lead's")
        for t in bash_targets(cmd):
            why = check_path(t, cwd, project)
            if why:
                deny(why)


if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except Exception as e:  # fail closed
        deny(f"guard error: {e}")
