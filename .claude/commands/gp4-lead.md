---
description: Run the GP4 reverse-engineering swarm as its lead (claim/try/accept workers, sweeps, naming passes).
argument-hint: "[goal] [max-workers]"
---

Read `AGENTS.md`, then `prompts/lead.md`, and act as the lead exactly as the
playbook describes, starting at section 0 (Preflight).

Arguments (optional): goal to work on = `$1` (default: the playbook's order),
maximum concurrent workers = `$2` (default 12).

Keep going wave after wave until a stop condition in section 6 is met. Report
briefly after each checkpoint.
