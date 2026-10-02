# AGENTS.md - GP4 reverse-engineering workspace

Canonical operating guide for every agent (Claude Code, Codex, ...) and human
working here. `CLAUDE.md` only points here.

**Goal:** recover Grand Prix 4 (`GP4.exe`, 2002, Geoff Crammond / Simergy,
32-bit x86) as readable C++ whose behaviour is *proven* equal to the original,
function by function, starting with the simulation core - then lift that into a
standalone library (`libgp4sim`).

## Map

| Path | What | Who writes |
|---|---|---|
| `Grand-Prix-4/` | the user's GP4 install - **read-only input, never modified, never committed** | nobody |
| `gp4re/` | the pipeline (`python -m gp4re ...`) | lead / humans |
| `state/gp4re.sqlite` | shared analysis + work ledger + names (WAL, atomic claims) | the tools |
| `analysis/` | generated fingerprint, function list, Ghidra exports | the tools |
| `src/gp4/{asm,c}/XXXX/<addr>.cpp` | accepted reconstructions, one function per file | `work accept` only |
| `src/gp4/runtime/` | registry bounds, `gp4_call_regs`, freestanding CRT shims | lead |
| `include/gp4/` | `gp4.h` conventions, `x87.h` bit-exact primitives, `types/` recovered layouts | lead |
| `specs/layouts/` | emulator input layouts for pointer arguments | lead |
| `build/scratch/<addr>/vK.cpp` | worker candidates | workers |
| `hook/` | in-game harness (`dinput8.dll` proxy) | lead |
| `prompts/worker.md`, `prompts/lead.md` | the two protocols | lead |
| `progress/` | committed snapshots: `regions.json`, `report.json`, `symbols.txt`, `ledger.txt` | `gp4re export` |
| `runtime/GP4/` | a COPY of the game for in-game testing | `gp4re runtime` |
| `docs/` | `STRATEGY.md` (why it is built this way), `RESEARCH.md` (the field, with sources) | humans |

## Roles

- **Lead** (one session, `/gp4-lead`, playbook `prompts/lead.md`): scopes goals,
  launches `gp4-worker` sub-agents in size bands, sweeps, settles names and
  layouts between batches, audits, exports, commits checkpoints.
- **Worker** (`.claude/agents/gp4-worker.md`, protocol `prompts/worker.md`):
  claim -> show -> write `build/scratch/<addr>/vK.cpp` -> `try && accept` | `defer`.
  Fenced by `tools/hooks/guard.py` to `build/scratch/`.

## The loop

```
python -m gp4re work claim --count 2 --max-size 48 --context   # atomic, leased 2h
python -m gp4re work show 0x00401d80                           # context packet
python -m gp4re work try 0x00401d80 build/scratch/00401d80/v1.cpp && \
python -m gp4re work accept 0x00401d80 build/scratch/00401d80/v1.cpp
python -m gp4re work defer 0x00401da2 "needs body layout" --needs "layout of the edx block"
```

Gates on every try (all must pass): **lint** (annotation + one `GP4_IMPL`,
banned constructs) -> **build** (MSVC x86, `/arch:IA32` x87) -> **shape**
(callees/imports/globals vs the original; tier C fails) -> **emu** (the
original and the reconstruction run on identical generated inputs in one
Unicorn address space; return values, declared register/x87/flag outputs and
every touched byte must match, bit-exact by default).

## Verification tiers

| Tier | Oracle | Proves |
|---|---|---|
| T0 | build + shape audit | it compiles and touches what the original touches |
| T1 | differential emulation (`gp4re/emu.py`) | same outputs and memory effects on generated inputs |
| T2 | in-game hooks (`hook/`, `gp4re runtime`) | the game still runs identically with the reconstruction swapped in; coverage + telemetry |
| T3 | (optional) byte matching with the original Visual C++ 6 toolchain, if the user supplies it | identical machine code for compiled C/C++ |

## Rules that survive compaction

1. `Grand-Prix-4/` is the user's install: never write, move or delete anything in it. Original
   bytes and assets are never committed (see `.gitignore`).
2. Only GP4's own code is analysed. Non-standard PE sections (third-party wrapper stubs) are
   out of scope; the tools exclude them and nobody works around that.
3. One goal at a time. Mechanical, behaviour-exact reconstruction first; renaming, struct
   recovery and modernisation are separate, later passes (MW2 post-mortem lesson).
4. The verifier decides, not the model: a function is done only when `work accept` prints
   ACCEPTED. Never claim parity from reading code.
5. Workers write only `build/scratch/`; only `work accept` writes `src/`; only the lead edits
   `include/`, `specs/`, `prompts/`, regions, names (`--force`) and exclusions.
6. Calls to unreconstructed GP4 functions go through their original addresses
   (`gp4::cdecl_<addr, R>`, `gp4::call_regs`); never re-implement a callee inline.
7. x87 fidelity: `gp4::x87::*` for transcendental/rounding instructions, original load/store
   widths and operation order; no `<math.h>`, no `volatile`, no inline asm in reconstructions.
8. Names need evidence (strings, callers, behaviour, community references like GP4PP).
   Without it keep `FUN_`/`DAT_`/`UnknownXX`.
9. Library code (CRT, D3DX, zlib, libpng, dxerr) and compiler artefacts are excluded, not
   reconstructed.
10. Defer honestly. An explained blocker is a good outcome; a forced pass is not.

## Known facts (verified 2026-10-02)

- `GP4.exe` sha256 `2046f390...4a926b12`, 6,275,072 bytes, image base `0x400000`, **no
  relocations** (fixed addresses: ideal for hooks and emulation), linker 6.0.
- 12,838 functions found; 10,528 game functions queued (3,127 asm track, 7,401 C track);
  2,310 library/compiler functions excluded up front.
- Same build GP4PP targets (its patch `0x488661`/`0x488668` matches byte for byte), so GP4PP's
  addresses and `CarDynamicData` layout (credited to Rene Smit and Paulo Blanco) are usable
  leads - verify before naming from them.
- The core runs the FPU at `0x037F` (set by its own `fninit` in `0x527453`; round-down
  mode `0x077F` for table lookups). The oracle uses it and runs `0x527453` first.
- Region map: `progress/regions.json`; details and evidence: `docs/STRATEGY.md`.
