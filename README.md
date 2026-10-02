# Reversible - an autonomous reverse-engineering pipeline for Grand Prix 4

Turns `GP4.exe` into readable C++, one function at a time, with every accepted
function **checked against the original on generated inputs** by an emulator-based oracle. The work is
done by a lead Claude session running a swarm of worker sub-agents. Workers
claim functions, write candidates, and only the verifier can accept them.

- How it works and why: [docs/STRATEGY.md](docs/STRATEGY.md)
- The field it builds on (Thief3, MW2, Burnout, Klonoa, Mizuchi, ...): [docs/RESEARCH.md](docs/RESEARCH.md)
- Rules for every agent: [AGENTS.md](AGENTS.md)

## Setup (Windows)

Requirements: Python 3.11+, Visual Studio (or Build Tools) with the C++ x86/x64
toolset, Git, and your own Grand Prix 4 install in `Grand-Prix-4/` (or `input/`).

```bash
python -m pip install -r requirements.txt
```

```bash
python -m gp4re analyze
```

```bash
python -m gp4re doctor
```

```bash
python -m gp4re status
```

`analyze` takes about 15 seconds. It fingerprints the exe, recovers about 12,800
functions, labels library and compiler code, maps the regions and fills the shared
ledger in `state/gp4re.sqlite`.

## Run the swarm

**Interactive.** Open this folder in Claude Code and run:

```
/gp4-lead core 12
```

The lead follows [prompts/lead.md](prompts/lead.md). It runs a smoke test, launches
`gp4-worker` sub-agents in size bands, refills their slots, sweeps results,
settles names and layouts between batches, and exports progress.

**Unattended** (session after session, each with a budget cap):

```bash
powershell -ExecutionPolicy Bypass -File tools/run_lead.ps1 -Sessions 4 -BudgetUsd 25 -Goal core
```

To stop after the current session, create `state/STOP`.

**By hand** (the same loop the workers use):

```bash
python -m gp4re work claim --count 1 --max-size 48 --context
```

Write `build/scratch/<addr>/v1.cpp`, then:

```bash
python -m gp4re work try 0x00401d80 build/scratch/00401d80/v1.cpp
```

## In-game verification (optional, last step)

```bash
python -m gp4re runtime install
```

Decompiling and verifying never needs the game running: the emulator runs the
original GP4 code itself. The game is only needed at the very end, to confirm
functions that talk to Windows or Direct3D inside the real game.

`runtime install` copies the game to `runtime/GP4/` (the original folder is never touched)
and installs the hook `dinput8.dll` with every accepted reconstruction switched
on. Launch the copy the way you normally launch GP4. Results land in
`runtime/GP4/gp4hook.log`, `coverage.log` and `telemetry.csv`. Set
`GP4RE_COVERAGE=1` before `install` to record which functions run in each phase
(press F9 at menu, session start and green light). The milestones (M1 to M5) are
in [docs/STRATEGY.md](docs/STRATEGY.md).

## Optional

- Ghidra pseudocode in the context packets: Ghidra 12.1.4 lives in `toolchain/`
  (checksum-verified, git-ignored). `python -m gp4re ghidra analyze` (about 75 s, once)
  then `python -m gp4re ghidra export` (all queued game functions, core first).
- Byte-matching track (T3): requires your own Visual C++ 6 toolchain. See STRATEGY section 2.

## Tests

```bash
python -m unittest discover -s tests -v
```

## Status (2026-10-02)

- Analysis: 12,838 functions recovered. 10,528 game functions are queued: 3,127
  on the assembly track and 7,401 on the C/C++ track. 2,310 library or compiler
  functions are excluded.
- Accepted: **33 functions** (32 in the simulation-core goal, 1 in legacy
  code). These cover sin/cos, fixed-point arithmetic, table search/lookup,
  state updates, field accessors and register-preserving stubs. The latest
  [ledger snapshot](progress/report.json) is the canonical count.
- Validation: **45 tests pass; 33/33 accepted functions pass regression with
  200 cases per function**. Coverage, missing inputs, ABI preservation and raw
  x87 outputs are checked. [Verification limits](docs/STRATEGY.md#acceptance-hardening-2026-10-02)
  distinguish generated-input evidence from a proof over all machine states.
- In-game harness: the hook DLL builds, and its register adapter passes the
  native tests, including all 64 arithmetic-flag combinations. It has not yet
  been run in-game (milestones M2/M3); the FPU-mode milestone M1 is settled statically.

Your game files and anything derived from them stay out of git. Keep this
repository private: the reconstructed source of a commercial game is not yours
to publish.
