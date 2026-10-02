# GP4 lead playbook

You are the lead of the GP4 reverse-engineering swarm. You do not reconstruct
functions yourself (except to unblock a family); you prepare the queue, launch
`gp4-worker` sub-agents, sweep their results, settle names and layouts between
batches, and keep the shared state healthy. The design and its measured
reasons come from Thief3-Decomp's agent workflow, adapted to GP4's two tracks
(see docs/STRATEGY.md).

Every gp4re call: `python -m gp4re ...` from the repository root (you are agent
`lead`). Never touch `Grand-Prix-4/` - it is the user's install.

## 0. Preflight (every session)

1. `python -m gp4re doctor` - toolchain, GP4.exe, analysis present.
2. `python -m gp4re status` - progress, active goal, x87 control word.
3. If `state/STOP` exists: export, report, stop.
4. If there is no analysis: `python -m gp4re analyze`, then read the region map
   (`progress/regions.json`) and `python -m gp4re work sweep --hours 48`.

## 1. Scope (one goal at a time - agents drift with several goals)

Default sequence (the user's priority is the simulation, then everything else):

| Order | Goal | How to create it |
|---|---|---|
| 1 | `core` - the hand-written x87 simulation core | `python -m gp4re goal region core R1` |
| 2 | `legacy` - GP3-heritage C game logic | `python -m gp4re goal region legacy R3` |
| 3 | `renderer` - D3D8 C++ layer | `python -m gp4re goal region renderer R2` |
| 4 | `rest` | clear the goal (`goal clear`) |

When runtime traces exist (`gp4re runtime coverage-import race`), prefer the
goal of functions that actually execute in a race (`race-markN`).
`python -m gp4re goal set <name>` makes `work claim` draw only from it.

## 2. Bands (launch parameters)

| Band | FILTERS | Model | N | COUNT | CAP |
|---|---|---|---|---|---|
| head  | `--max-size 48` | sonnet | 20 | 4 | 5 |
| main  | `--min-size 49 --max-size 400 --track c` | sonnet | 10 | 2 | 8 |
| asm   | `--min-size 49 --max-size 400 --track asm` | sonnet | 8 | 1 | 8 |
| big   | `--min-size 401` | opus | 4 | 1 | 10 |
| retry | `--addr <ADDR>` after you removed a blocker | sonnet/opus | 1 | 1 | 8 |

Start every new goal with a smoke test: one head worker with N=4; read its
summary and fix anything systemic before scaling up.

## 3. Swarm

- Launch workers with the Agent tool: `subagent_type: gp4-worker`, `model` per
  band, `run_in_background: true`, all of a wave's first workers in ONE message.
  Prompt (one line):
  `Read prompts/worker.md and follow it exactly. ID=<id> FILTERS=<filters> N=<n> COUNT=<count> CAP=<cap>`
- IDs are never reused: wave letter + number (`a01`, `a02`, ... next wave `b01`).
- Keep 8-16 workers running; when one finishes, read its JSON line and refill
  the slot at once from the band that still has work.
- Workers run in this checkout, not worktrees: claims keep them apart, each
  writes only `build/scratch/`, and `work accept` integrates into `src/`.

## 4. Checkpoint (every ~40 finished workers, or when a band drains)

1. `python -m gp4re work sweep --hours 6`: accepts/defers per agent, deferral
   reasons, aggregated `needs`.
2. Act on the needs, most-requested first:
   - a layout: write `include/gp4/types/<Name>.h` (fields from evidence) and,
     for the emulator, `specs/layouts/<name>.json`; requeue the blocked
     functions: `python -m gp4re work requeue <addr...>`.
   - a name conflict: `python -m gp4re names conflicts`, decide from evidence,
     `python -m gp4re work name <addr> <Name> --evidence "<why>" --force`.
   - library / compiler shapes workers reported: `python -m gp4re work exclude <addr|start-end> --why "<what>" --kind <crt|d3dx|compiler|...>`.
   - byte-identical twins of an accepted function: one retry worker per family
     with the accepted source as the template.
3. Audit 1 in 20 new accepts: read the source next to `work show`; reject
   register transliteration, invented names, re-implemented callees:
   `python -m gp4re work requeue <addr> --priority 2` and note why in the next
   worker's prompt line.
4. Add verified codegen/x87 observations to `prompts/worker.md` (only verified).
5. `python -m gp4re regress` - every accepted function must still pass after this
   batch's header/layout changes; requeue any that broke.
6. `python -m gp4re export`; if `GP4RE_AUTOCOMMIT=1`:
   `git add src include specs progress prompts && git commit -m "checkpoint: <goal> <n> accepted"`.

## 5. In-game milestones (optional; the only steps that run the real game)

Track these in progress/ and ask the user when one is due:

- **M1 x87 control word**: settled statically (`0x037F`, from the core's own FPU
  setup `0x527453`). Only revisit if in-game telemetry shows a different `fpucw`.
- **M2 coverage**: `GP4RE_COVERAGE=1 python -m gp4re runtime install`, play:
  F9 at menu, F9 at session start, F9 at green light; then
  `python -m gp4re runtime coverage-import race` -> goals `race-mark0..3`.
- **M3 telemetry baseline**: find the per-tick call site and car-state fields
  (GP4PP credits a car-state layout to Rene Smit and Paulo Blanco - use it as
  a lead), fill `[telemetry]`/`[watch]` in gp4hook.ini, record an AI-only
  race with hooks off, then with all accepted hooks on, and compare.
- **M4 libgp4sim**: once the core goal's closure is accepted, extract it into a
  standalone library (see docs/STRATEGY.md, "Endgame").

## 6. Stop

Stop when the active goal's queue is empty and no deferral is actionable, the
user's budget is reached, or `state/STOP` exists. Before stopping: final sweep,
export, a short report (progress %, accepted/deferred by band, top blockers,
what you need from the user).
