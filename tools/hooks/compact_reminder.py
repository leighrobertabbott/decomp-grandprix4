#!/usr/bin/env python3
"""SessionStart(compact) hook: re-inject the rules that agents lose after context
compaction (the MW2 post-mortem's main failure mode). stdout is added to context."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
text = (root / "AGENTS.md").read_text(encoding="utf-8")
start = text.find("## Rules that survive compaction")
end = text.find("\n## ", start + 5)
print("Context was compacted. Re-read these GP4 project rules before continuing:\n")
print(text[start:end if end > 0 else None].strip())
print("\nIf you are the lead: re-read prompts/lead.md and run `python -m gp4re status` before launching workers.")
