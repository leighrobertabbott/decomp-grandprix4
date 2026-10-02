# Strategy: reverse engineering Grand Prix 4 with an agent swarm

Why the pipeline is built the way it is, what is known about the binary, and
the milestones to the end goal. Everything stated as fact here was measured on
the user's `GP4.exe` on 2026-10-02; hypotheses are marked as such.

## 1. The binary

| Fact | Value | Consequence |
|---|---|---|
| File | `GP4.exe`, 6,275,072 bytes, sha256 `2046f3902246b2aff48dca2e4f7e8c049d410166e60a5dc20fe3750f4a926b12` | addresses in this repo are for this build only (`progress/regions.json` is keyed by the hash) |
| Build | same build GP4PP (Oggo87) patches - its `0x488661`/`0x488668` patch site matches byte for byte | community addresses/layouts apply directly |
| Image base | `0x400000`, **no relocation table** | every address is fixed: perfect for jmp-patch hooks and for emulation |
| Code | `.text` 2,025,526 bytes; entropy 6.74 (plain code) | readable statically; no unpacking involved |
| Out of scope | `stxt774`, `stxt371` (non-standard sections of a third-party wrapper) | excluded from analysis, emulation and patching by design |
| APIs | D3D8, DirectInput8, DirectSound, Bink, winmm, wsock32, GDI/user32, advapi32 | rendering/IO is a thin layer over a self-contained simulation |

### Rich header (which tools built the objects)

| prodid | build | count | reading |
|---|---|---|---|
| 10 (Utc12_C) | 8047 | 175 | Visual C++ 6 C - most plausibly the VC6 C runtime |
| 11 (Utc12_CPP) | 8047 / 8966 / 8168 / 8569 | 12 / 56 / 1 / 1 | Visual C++ 6 C++ |
| 14 (Masm613) | 7299 | 39 | MASM 6.13 objects (CRT asm helpers and/or game assembly) |
| 18 (Masm614) | 8444 | 1 | one newer MASM object |
| 28 / 29 (Utc13 C / C++) | 9178 | 3 / 82 | pre-release Visual C++ 7 - consistent with the DirectX 8.x SDK's static D3DX library |
| 49 (0x31) | 9044 | 193 | per public comp.id tables Utc12_2_CPP: VC6 SP5 + Processor Pack C++ - hypothesis: the game's own newer C++ |
| 0 / 1 / 6 / 12 / 25 | - | - | unmarked objects, imports, resources, alias objects, import libs |

Linker 6.0. The one thing that matters operationally: **the game was built with
VC6-era compilers plus hand-written assembly**, so byte-matching needs the
user's own VC6 toolchain (track T3, optional), while semantic equivalence works
today with any modern MSVC.

### Region map (fingerprints + evidence)

Function alignment and inter-function padding change sharply at object-group
boundaries, which splits `.text` into contiguous regions:

| Region | Range | Fingerprint | Evidence | Game functions (asm / c track) |
|---|---|---|---|---|
| R1 asm core | `0x401000-0x440000` | unaligned, unpadded; ~60% of functions consume registers/flags/x87 stack | essentially no strings or imports; x87 everywhere (e.g. `0x401d80` builds sin/cos pairs from three double angles via `edx`) | 1,666 (1,005 / 661) |
| R2 renderer C++ | `0x440000-0x510000` | 16-byte aligned, nop padded, thiscall-heavy | `CGP4CarShader`, `CSkyDome`, `CGP4Cockpit`, HUD ids, memory manager; GP4PP's wheel-shader patch | 2,772 (470 / 2,302) |
| R3 legacy C | `0x510000-0x560000` | unaligned C + asm | `f1gstate.dat`, `gconfig.txt`, `circuits\*.dat`, `.jam` textures, `D:\grand prix 3`, GP3 2000 network/2-player | 3,601 (1,162 / 2,439) |
| R4 libraries | `0x560000-0x5a4000` | mixed | D3DX mesh/validation, d3dxof, CPU-specific SIMD (`DisableD3DXPSGP`), libpng 1.0.5, zlib | excluded (1,137) |
| R5 mixed C++ | `0x5a4000-0x5d8000` | 16-byte aligned like R2 | plus dxerr tables (`D3DERR_*`, `DMUS_S_*`) and zlib inflate strings | 1,602 (199 / 1,403) - needs library triage |
| R6 CRT tail | `0x5d8000-0x5f0000` | CRT asm padding | entry `0x5ddec6`, `Runtime Error!`, heap selection | 887 still queued - lead should confirm and exclude |

Totals: 12,838 functions; 10,528 game functions queued; 2,310 excluded up front;
415 game functions with 20+ x87 instructions; 1,869 game functions have a
byte-identical twin (modulo addresses) - e.g. `0x401d80` and `0x51edf9`, an asm
routine duplicated between the core and the GP3-heritage code.

Known imprecision: function bodies are recovered by recursive descent; where a
no-return call or shared tail is missed, a body can run into its neighbour
(sizes then overlap, which inflates byte totals). The shape gate and emulator
are per-function and unaffected; a Ghidra pass or worker reports fix the rest.

## 2. Why two tracks

Byte-matching decompilation (Snowboard Kids, Thief3, LEGO Island/reccmp) is the
gold standard *for compiled code with the original compiler*. GP4 breaks both
assumptions: Crammond wrote the physics and AI in assembler (2002 interview),
and the VC6 toolchain is not freely available. Thief3's workers explicitly
defer inline-asm targets - for GP4 that would defer the most important code.

So equivalence is defined behaviourally, per function:

- **asm track** (style `asm-like`): the reconstruction is
  `void fn(gp4::Regs*)`; the oracle feeds the original routine and the
  reconstruction identical registers, flags, x87 stack and memory, and demands
  identical outputs and memory effects. Bit-exact x87 is achievable because
  reconstructions use the same instructions (`include/gp4/x87.h`) and the
  oracle pins the FPU control word.
- **c track**: normal C/C++ with the original calling convention, same oracle,
  plus the shape audit. If the user later supplies VC6, the same sources can be
  pushed toward byte matching (T3) with reccmp-style annotations already in place
  (`// FUNCTION: GP4 0x...`).

## 3. Oracles (what makes autonomy safe)

The lesson repeated across every project studied: agents claim parity they do
not have (Chromatron: "LLMs were very eager to say something is the same");
only a mechanical comparator keeps them honest. Here:

1. **Shape audit** - catches stubbed or re-routed bodies regardless of inputs.
2. **Differential emulation** - GP4.exe and the reconstruction DLL share one
   Unicorn address space; unreconstructed callees run as original code, Win32
   calls trap as "external". Verified: a sin/cos swap and a wrong flag bit are
   both caught with exact addresses and values; correct versions pass 200/200.
3. **In-game hooks** (`hook/`): the `dinput8.dll` proxy jmp-patches accepted
   functions into a copy of the game (toggle per function in `gp4hook.ini`),
   adapting register conventions through a tested thunk; it also records
   first-hit coverage per phase (F9 markers) and per-tick telemetry from a
   patched call site, including the live x87 control word.
4. **Telemetry diff** (M3) - same AI-only session with hooks off vs on.

### Acceptance hardening (2026-10-02)

The first Codex smoke batch exposed that several configured checks were not
implemented. Executable synthetic counterexamples demonstrated false passes
for omitted register outputs, fixed-canary hidden inputs, clobbered ABI
registers and signed zero. These are now regression tests, alongside coverage,
caller-stack, raw x87 precision and out-of-scope execution checks.

Full T1 acceptance requires every generated original case to be valid, the
configured target-body block/branch coverage, and non-interference checks for
omitted GPR, arithmetic-flag and stack inputs. The complete register block is
compared for register-convention functions, because the live hook restores all
of it. Compiled functions also compare callee-saved registers and stack balance.
Caller-visible memory is byte-exact; private local/deallocated callee-stack
bytes are outside the contract because the reconstructed compiler uses a
different local layout. The hook now saves incoming flags without first
changing them; native tests exercise all 64 arithmetic-flag combinations.

Raw 80-bit x87 outputs are captured before conversion. A register-convention
output that cannot survive its `double` adapter slot exactly is deferred.
The current adapter cannot supply arbitrary extended-precision inputs or
represent every x87 status/control-state effect. Global fields can be varied
through evidenced layouts, but omitted global inputs, pointer aliases and
MMX/XMM state are not yet covered by the input-completeness checks. Structural
calls/imports must match even for trivial functions; runtime skip requires a
named import reachable in the original call graph.

T1 is evidence over generated inputs and measured coverage, not a mathematical
proof for all possible machine states. Full reconstruction remains incomplete
until all in-scope functions and runtime-dependent checks are resolved.

Some arithmetic flags are architecturally undefined (for example OF after a
multi-bit shift). Reconstructions currently match the pinned Unicorn 2.1.4
model for these bits; this does not establish equality on every native CPU.
The SHRD-by-12 observation is checked over 134 synthetic input pairs and comes
from the oracle's [shift translation](https://raw.githubusercontent.com/unicorn-engine/unicorn/2.1.4/qemu/target/i386/translate.c)
and [condition-code helpers](https://raw.githubusercontent.com/unicorn-engine/unicorn/2.1.4/qemu/target/i386/cc_helper_template.h).
Its OF uses the sign-bit difference between the concatenated value shifted by
11 and by 12. Native flag consumption and in-game behavior remain part of T2.

Two native bugs were caught by `tests/native/test_adapter.cpp` before they
could crash the game (flags clobbered by `add esp`; reserved `st` name).

### The x87 control word (settled statically)

The core sets its own FPU modes, so no game run was needed to find them. Its
setup routine `0x527453` executes `fninit` (control word `0x037F`: 64-bit
internal precision, round-to-nearest), stores that as the "normal" mode in
`0x6f29a6`, and stores a round-down variant (`0x077F`) in `0x6f29a4`. Core
routines switch between the two: the table-lookup-with-interpolation routine
`0x411891` uses round-down to pick the table slot, then restores the normal
mode. So:

- the oracle runs at `0x037F` (`config/project.toml`, `[oracle] fpu_cw`);
- the emulator runs GP4's own initialisers first (`[oracle] init_calls`), so
  globals like these two control-word variables hold their real values instead
  of the zeros in the file.

`gp4re fpucw <value>` overrides the default if in-game logs ever disagree.

## 4. Orchestration (adopted from measured results)

- Lead + Agent-tool sub-agents, not headless `claude -p` workers: Thief3
  measured ~2M input tokens per match for headless workers (each request carried
  ~63K of default context) against sub-agents starting at ~12.6K.
- Size bands (`prompts/lead.md`): tiny functions in big cheap batches, large
  ones to stronger models with higher caps.
- Library and compiler-generated code excluded before batches; names and
  layouts settled between batches (one wrong callee name blocks many callers).
- Claims re-checked under the lock; leases expire after 2 hours.
- One goal at a time; rules re-injected after compaction (MW2 post-mortem).
- Workers fenced to `build/scratch/` by a fail-closed PreToolUse guard.

## 5. Milestones

| | Milestone | Done when |
|---|---|---|
| M0 | Pipeline + first accepted functions | done: analysis loaded, both tracks proven on real code (`0x401d80` asm, `0x510b48` C) |
| M1 | x87 control word | done statically: `0x037F` from the core's own FPU setup (`0x527453`) |
| M2 | Runtime coverage | goals `race-mark*` imported; queue scoped to code that runs in a race |
| M3 | Telemetry baseline + per-tick call site + car state layout (seed from GP4PP `CarDynamicData`) | hooks-off vs hooks-on AI race diff is empty |
| M4 | Simulation core closure accepted | every function reachable from the sim tick accepted (goal `core` + race coverage) |
| M5 | `libgp4sim` | the accepted closure builds as a standalone library: inputs (controls, track), `step(dt)`, outputs (car states) - the libsm64 pattern |
| M6 | Modernisation | structs, names, classes, portable math - only after M4, as separate passes |

## 6. Honest limits

- No public system turns `GP4.exe` into Crammond's original source; the
  output is equivalent, readable source, proven per function.
- Functions that touch Win32/D3D only pass T1 as "pending runtime"; confirming
  them needs the game running. That is the only part that needs the game, it
  comes last, and a script can launch it. Everything else is programmatic.
- Throughput expectations from comparable projects: MW2 reached 34% of
  functions in ~4 weeks with 4 agents; Thief3's swarms ran 72-97% match rates
  per band. GP4's asm track has no precedent at this scale.

## 7. Legal and distribution

The input is the user's own copy. Original bytes, assets and generated dumps
never enter git (`.gitignore`). Reconstructed source of a commercial game is
a legal grey area to publish; keep the repository private unless the user
decides otherwise. Community tools used as references (GP4PP, GP4MemLib) are
MIT-licensed; credit them when importing layouts.
