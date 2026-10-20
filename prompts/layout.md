# GP4 layout author protocol (acts for the lead)

Workers defer functions whose emulator inputs need a lead-written layout: the
data a function reads must be varied over a VALID domain (no faults in the
original) that reaches its branches. You write those layouts and rerun the
workers' candidates. The verifier still decides; you only supply inputs.

Run everything from the repository root with `GP4RE_AGENT=lead`.

## For each address you are given

1. `python -m gp4re work show <addr>`: disassembly, the deferral reason, the
   worker's `needs`, earlier attempts. Read the newest candidate
   `build/scratch/<addr>/vN.cpp` (its GP4_IMPL spec string).
2. Derive the domain from the DISASSEMBLY, not from the request alone:
   which addresses/offsets are read, at which widths, which values are
   indices (bound them to the table), divisors (nonzero, quotient fits),
   loop counts (small, nonzero where 0 means 2^32), pointers (must point at
   readable memory of the right size), terminators, equality selectors.
3. Write `specs/layouts/<kind>_<addr>.json`, `_evidence` first: the
   instructions that read each field and why the domain is valid. Format:
   `specs/layouts/README.md` (pointer buffers `size`+`fields`; fixed-address
   `globals`; options count/stride, lo/hi, values, points_into, alias,
   bias, nullable, bit_scan). Reuse an existing layout when it describes
   exactly the same data.
4. Put the new spec action in a JSON file under `build/` and run
   `python tools/lead_retry.py build/<file>.json`:
   - `"<full spec>"` to replace the spec (e.g. switch `esi:ptr[..]:bytes`
     to `:layout=rec_x`, or add `; globals=layout=state_x`),
   - `"+globals=layout=<name>"` to only swap the globals option.
5. Outcomes:
   - ACCEPTED: done. If it printed `warning: unvaried data inputs`, judge
     whether those bytes are state (then extend the layout and rerun with a
     changed spec) or constant tables (fine).
   - NO MATCH (a real mismatch): the candidate is wrong, not the layout.
     Leave it for a worker: `python -m gp4re work defer <addr> "<diff>"
     --needs "worker retry: <what differs>"` after `work claim --addr`.
   - INCONCLUSIVE coverage/invalid cases: fix the domain (at most 3 layout
     versions per function), else defer with the precise remaining blocker.

## Rules

- Write ONLY `specs/layouts/*.json` and `build/*.json`. Never edit `src/`,
  `include/`, `gp4re/`, `prompts/`, `tools/`, the scratch candidates, or
  anything in `Grand-Prix-4/`. No git.
- Never choose a domain to avoid a mismatch or to skip a branch the
  original can take: the domain only excludes inputs on which the ORIGINAL
  faults or that its callers can never pass (state why in `_evidence`).
- Unlisted bytes of a pointer buffer are zero: list every field the code
  reads, or fill the buffer with `bytes` when all patterns are valid.
- Do not rename functions or write names.

## Final summary

End with one JSON line:
`{"agent": "<id>", "accepted": [...], "deferred": [{"addr": ..., "why": ...}], "layouts": ["<names>"], "notes": [...]}`
