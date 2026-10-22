# Grand Prix 4 Decompilation

A work-in-progress decompilation of **Grand Prix 4**, Geoff Crammond's 2002 Formula One
simulation for Windows (Simergy). Functions of `GP4.exe` are reconstructed as C++, one
per file, and each is accepted only when it behaves bit for bit like the original: both
run in the same emulator on generated inputs that reach at least 90% of the original's
basic blocks, and every return value, register, x87 and flag output and memory write must
match.
Functions that depend on Windows calls the emulator does not model are accepted
provisionally, pending a check inside the running game. The reconstructions are mechanical
for now (register-level for the assembly core); naming and structure recovery come later.

What exists today: about a quarter of the game's own code, most of the simulation core
and essentially none of the renderer (see [Progress](#progress)). This does not build a
playable game or a replacement `GP4.exe`: each reconstruction is tested in an emulator
against your original executable, and can be patched into a copy of it. This is a
behavioural decompilation; reproducing the original machine code byte for byte is not a
goal.

The repository contains no original game files: no executable, assets, data or
disassembly dumps. You need your own copy of the game.

**Goal:** recover the simulation core first and lift it into a standalone library
(`libgp4sim`: controls and track in, `step(dt)`, car states out), while the rest of the
game follows.

## Target

| | |
|---|---|
| Game | Grand Prix 4 (2002), Geoff Crammond / Simergy |
| Platform | Windows PC, 32-bit x86 (PE32, Windows GUI), Direct3D 8 |
| Executable | `GP4.exe`, 6,275,072 bytes |
| SHA-256 | `2046f3902246b2aff48dca2e4f7e8c049d410166e60a5dc20fe3750f4a926b12` |
| MD5 | `eb6d756ad1a17e8f8b6f77177507b550` |
| Image base | `0x400000`, no relocation table (every address is fixed) |
| Entry point | `0x5ddec6`, Visual C++ 6 CRT start-up (presumably `WinMainCRTStartup`: GUI subsystem, CRT imports) |
| Code | `.text` at `0x401000`, 2,025,526 bytes, entropy 6.74 (plain, unpacked code) |
| Game code | 10,135 functions classified as game code (summed sizes 1,682,784 bytes, including overlapping bodies and the not yet triaged R5/R6 functions) |
| Built with | Visual C++ 6 compilers (including SP5 + Processor Pack), pre-release Visual C++ 7 objects (likely static D3DX), MASM 6.13 / 6.14, linker 6.0 (Rich header) |
| Same build as | the one targeted by [GP4PP](https://github.com/Oggo87/GP4PP), Oggo87's community patcher (its `0x488661`/`0x488668` patch site matches byte for byte) |
| Out of scope | `stxt774`, `stxt371` (non-standard sections of a third-party wrapper) |

Every address in this repository is for this build only. `python -m gp4re analyze`
refuses any other `GP4.exe` unless given `--allow-other-exe`.

## Progress

<!-- status:begin -->

![code](https://img.shields.io/badge/code-27.35%25-blue) ![verified](https://img.shields.io/badge/verified-4%2C624_functions-brightgreen) ![pending in-game](https://img.shields.io/badge/pending_in--game-197-yellow) ![deferred](https://img.shields.io/badge/deferred-71-orange) ![todo](https://img.shields.io/badge/todo-3%2C635-lightgrey)

**27.35%** of GP4.exe's game code (432,362 of 1,580,985 bytes) is reconstructed and accepted: **4,624** functions verified by differential emulation, **197** accepted pending the in-game check, 71 deferred with a recorded blocker, 3,635 still to do.

| Region | Address range | Verified | Pending in-game | Deferred | To do | Code accepted |
|---|---|---:|---:|---:|---:|---:|
| Simulation core (hand-written x87 assembly) | `0x00401000`-`0x00446160` | 1,846 | 3 | 16 | 27 | 93.8% |
| Renderer and front end (Direct3D 8 C++) | `0x00446160`-`0x00510000` | 434 | 17 | 20 | 1,511 | 2.0% |
| Game logic (GP3-heritage C and assembly) | `0x00510000`-`0x00560000` | 2,120 | 170 | 22 | 987 | 49.3% |
| Libraries: D3DX, libpng, zlib (excluded) | `0x00560000`-`0x005a4000` | 0 | 0 | 0 | 0 | excluded |
| Mixed C++ and DirectX error tables | `0x005a4000`-`0x005da800` | 224 | 7 | 13 | 1,110 | 2.7% |
| Visual C++ 6 runtime | `0x005da800`-`0x005f0000` | 0 | 0 | 0 | 0 | excluded |

<sub>Generated 2026-10-22 from the ledger by `python tools/readme_status.py`; per-function state is in [progress/ledger.txt](progress/ledger.txt).</sub>

<!-- status:end -->

### How progress is measured

- The headline figure is accepted function bytes (every function in state `accepted` or
  `accepted-runtime`) over the summed sizes of all functions classified as game code
  (1,682,784 bytes). Library, compiler and data functions are left out. Sizes are summed,
  so bodies that overlap a neighbour are counted twice; the union of game-function bytes
  is 1,584,516.
- The function counts above total 10,138: the 10,135 game functions less one excluded as
  unreachable, plus four D3DX functions that were reconstructed. The renderer (R2) has not
  been started; its 54 accepted functions are 1-byte routines.
- R5 and R6 hold 2,488 functions still to do (about 272 KB) that have not been triaged
  for library code yet. Library and runtime code found there leaves the denominator,
  which will raise the percentage.
- The figures come from the maintainer's ledger (`state/gp4re.sqlite`, not in git).
  [progress/ledger.txt](progress/ledger.txt) lists every accepted, accepted-runtime,
  deferred and excluded function (functions not listed are still to do), and
  [progress/report.json](progress/report.json) holds the totals.

### Matching policy

This is a **behavioural** decompilation, not a byte-matching one. The simulation core is
hand-written assembly with no compiler output to match, and Visual C++ 6 is not freely
available, so equivalence is defined per function by differential emulation: bit-exact on
generated inputs. See [Verifying a function](#verifying-a-function) and
[docs/STRATEGY.md](docs/STRATEGY.md) section 3 for what that does and does not prove.

## How the game is built

`GP4.exe` is an unpacked 32-bit Windows executable with a fixed image base and no
relocation table, so every address is the same on every run. That makes it well suited
to static analysis, emulation and `jmp`-patch hooks. It imports 246 functions from 12
DLLs: Direct3D 8 (`Direct3DCreate8`), DirectInput 8 (`DirectInput8Create`), DirectSound
(by ordinal), Bink video, winmm, wsock32, GDI, user32, advapi32, ole32, winspool and
kernel32. No function in the simulation core (R1) or the GP3-heritage code (R3) calls an
imported function directly: Windows and DirectX are reached through the C++ code (R2, R5),
the libraries and the runtime.

The Rich header records which tools built the linked objects:

| Tool | Objects | Reading |
|---|---:|---|
| Visual C++ 6 C (Utc12_C 8047) | 175 | most plausibly the VC6 C runtime |
| Visual C++ 6 C++ (Utc12_CPP) | 70 | VC6 C++ |
| VC6 SP5 + Processor Pack C++ (prodid 49, build 9044) | 193 | hypothesis: the game's own newer C++ |
| MASM 6.13 / 6.14 | 39 / 1 | CRT assembly helpers and/or the game's assembly |
| pre-release Visual C++ 7 C / C++ (Utc13 9178) | 3 / 82 | consistent with the DirectX 8.x SDK's static D3DX |
| Linker | 6.0 | |

Function alignment and inter-function padding change sharply at object-group boundaries,
which splits `.text` into contiguous regions ([progress/regions.json](progress/regions.json)):

| Region | Range | What it is |
|---|---|---|
| R1 core | `0x401000`-`0x446160` | Simulation core (physics, AI, maths): hand-written x87 assembly. Unaligned and unpadded; ~60% of functions take register, flag or x87-stack inputs; essentially no strings or imports. *Likely.* |
| R2 renderer | `0x446160`-`0x510000` | Direct3D 8 renderer and front end: 16-byte-aligned, nop-padded, thiscall-heavy C++ (`CGP4*Shader`, `CSkyDome`, `CGP4Cockpit`, HUD text ids, memory manager). *Likely.* |
| R3 game logic | `0x510000`-`0x560000` | Unaligned C mixed with assembly: game-state save/load (`f1gstate.dat`), `gconfig.txt`, `circuits\*.dat`, `.jam` textures, a `D:\grand prix 3` path and GP3 2000 network/two-player strings. *Likely.* |
| R4 libraries | `0x560000`-`0x5a4000` | D3DX mesh code, DirectX-file loader, CPU-specific SIMD pipeline, libpng 1.0.5, zlib. *Provisional.* |
| R5 mixed | `0x5a4000`-`0x5d8000` | C++ aligned like R2, plus dxerr8 error tables and zlib inflate strings. *Provisional; not yet triaged for library code.* |
| R6 CRT | `0x5d8000`-`0x5f0000` | Visual C++ 6 runtime tail (entry point, heap selection, locale) with a few D3DX/game functions interleaved. *Provisional; CRT functions are excluded as they are identified.* |

R3's strings, and the assembly it shares with R1 (identical modulo addresses), point to
code carried over from Grand Prix 3; that reading rests on this evidence alone. 359 game
functions have 20 or more x87 instructions, and 1,836 have a twin elsewhere in the binary
that is identical modulo addresses.

**The x87 control word.** The game sets its own FPU modes. Its FPU setup routine
`0x527453` (in R3) executes `fninit` (control word `0x037F`: 64-bit internal precision,
round-to-nearest), stores that as the normal mode at `0x6f29a6`, and stores a round-down
variant `0x077F` at `0x6f29a4`. Core routines switch between the two: the
table-lookup-with-interpolation routine `0x411891` rounds down to pick the table slot,
then restores the normal mode.

Background, evidence and milestones: [docs/STRATEGY.md](docs/STRATEGY.md).

## Inside GP4.exe

What the reconstruction has shown so far about how GP4 works. Addresses, record sizes and
counts are read from the code; what fields mean is a working interpretation, and every
name is provisional. Only function behaviour is verified. Evidence for each item is in
[specs/layouts/](specs/layouts/).

- **Two angle formats.** 32-bit fixed-point angles (a full turn is 2^32 units) are
  converted to doubles; 16-bit car headings index a Q14 sine/cosine table. Beside the x87
  code, much of the core is integer fixed-point maths.
- **GP3 heritage.** The core and the GP3-heritage code share routines that are identical
  modulo addresses, and R3 still keeps a `D:\grand prix 3` root path and a CD-ROM scan.
- **Leftovers.** `0x51bef2` is an unreferenced RGB/HSV colour-conversion self-test.
- **Car records.** 22 records of `0x41c` bytes at `0x66fef0` hold position, heading,
  current track segment, car number, gear, engine speed and road speed.
- **Tyres.** Each wheel's force follows a curve of the shape of Pacejka's Magic Formula,
  scaled by a surface-grip term and a tyre-load factor.
- **Wheels and mass.** Four `0x300`-byte wheel records are integrated by one routine; a
  23-part mass model computes the car's centroid and inertia sums.
- **The track is a ring** of `0x160`-byte segment records at `0x8943a0`, with the pit
  lane branching off it.
- **Grid and racing line.** The grid places cars by row on the segments after a start
  segment; a 96-entry lane table is cleared per circuit and filled by what looks like the
  racing-line pass.
- **Circuits, setups and lap records.** 17 circuits, per-circuit setups for 22 drivers,
  and 17 lap records merged from `RECORDS.REC`.
- **Clocks and pit stops.** A free-running millisecond race clock is compared with per-car
  deadlines, including the pit-stop timer.
- **Text and menus.** All UI text goes through one pointer table filled at load time and
  expanded by a small byte-code interpreter; menus are static data structures.
- **Sound.** 16 channels and 100 sample records over DirectSound.
- **Input.** A key-state bitmap, a key queue, eight controller presets and six DirectInput
  device slots.
- **Replay and network** appear to share one opcode stream.
- **Saves and services.** A central service dispatcher handles save-state, packet and
  temp-file commands.

<details>
<summary>Address-level notes</summary>

- **Two angle formats.** 32-bit angles are turned into doubles by multiplying with
  pi/2^31 (e.g. `0x401d5c`). The six angle-scale doubles at `0x6f2bbc`-`0x6f2be4` (2^31/pi
  at `0x6f2be4`, pi/2^31 at `0x6f2bdc`) are zero in the file and computed from pi at
  start-up by `0x527476`. Car headings are 16-bit words, looked up in a Q14 sine/cosine
  table at `0x7c5c5c` (byte offsets up to `0x2000`) that no code writes, with index
  `(|a| >> 2) & 0xfffe`. The fixed-point maths uses Q14 gains via `imul`/`shrd`.
- **GP3 heritage.** Routines identical modulo addresses in both the core (R1) and R3:
  `0x401d80`/`0x51edf9` (sin and cos of three angles), `0x401da2`/`0x51ee1b` (rotation
  matrix from those pairs), `0x401d5c`/`0x51edd5` (fixed-point angle to double). The
  `D:\grand prix 3` root path (`0x81e5d9`) is chosen by `0x5367b9`/`0x5368d6`; `0x5370bd`
  scans for a CD-ROM drive and writes its letter over the path's first character.
- **Leftovers.** `0x51bef2` runs fixed loops through `0x51b1f9` (RGB to HSV) and
  `0x51b2ea` (HSV to RGB); nothing calls it.
- **Car records.** 22 records of `0x41c` bytes at `0x66fef0`. `0x409162` zeroes a record
  and stores its index. Dwords `+0`/`+4` are the x/z position, word `+0xe` the heading,
  `+0x1c` the current track-segment pointer, byte `+0x7c` the car number plus flag bits. A
  car-order list of record offsets at `0x66fe3c` is built by `0x4091b5`. Whether this is
  GP4PP's `CarDynamicData` has not been checked.
- **Drivetrain.** Car byte `+0x72` is the current gear (-1 reverse, 0 neutral, 1..7),
  byte `+0x1cd` the gear count (`0x41034e` stores 6 or 7), word `+0x62` engine speed and
  word `+0x9a` road speed; `0x4161fb` looks like the gear-shift decision. The rev limits at
  `0x623c2c` (100 rpm << 8) and `0x623c30` (20,000 rpm << 8) are never stored by game code.
- **Tyres.** Four `0x158`-byte tyre records at `0x601510` (front tables for wheels 0-1,
  rear for 2-3). `0x411e17` computes `D sin(C atan(Bx - E(Bx - atan(Bx))))`, the shape of
  Pacejka's "Magic Formula" tyre model, from four doubles at `+0`/`+8`/`+0x10`/`+0x18`
  (read as B, C, D, E). `0x41724d` scales the resulting forces per wheel by a surface-grip
  term and a tyre-load factor.
- **Wheels and mass.** The four `0x300`-byte wheel records at `0x6380d8` are integrated by
  `0x4200a7` with a step value; `0x419d76` combines a vector from each record through nine
  Q12 coefficients at `0x638d6c`-`0x638d8c`. A 23-part mass model: `0x444517`
  (`Parts_ComputeMassProperties`) takes a 23-bit part mask in EAX, writes the centroid to
  `0x62a064`-`0x62a06c` and forms parallel-axis inertia sums from stride-`0x10` part
  tables at `0x62a424`, `0x62a5ac` and `0x62a7cc`.
- **The track is a ring.** `0x160`-byte segment records at `0x8943a0`, with lap-end
  sentinel `[0x866ecc]`, pit start `[0x891500]`, pit end `[0x866ef8]`, pit entry
  `[0x866f00]` and pit exit `[0x866f04]`. Walks cross boundaries only through bit 3 of byte
  `+0x92`. Wrap helper `0x406051` (forward boundary remap) and step-back helper `0x406077`,
  plus `0x4060b0`/`0x4060e6`, which also enter and leave the pit lane; side tests
  `0x405ad0` and `0x405e19`.
- **Grid and racing line.** `0x408d6d` places the starting grid; a reset car goes on
  segment `[0x866f08] + 1 + 3*row` (+1 for an even car number), with the row in car byte
  `+0xfd` (0..11). A lane table at `0x6269b8` holds 96 16-byte entries (four words, then
  two dwords that are 0 or a segment-record pointer); `0x40cd8b` clears it per circuit and
  `0x43ff93` fills it. What the words mean is not settled yet.
- **Circuits, setups and lap records.** `[0x7ad894]` is the current circuit, 0..16 (17
  circuits). Setup entries for 22 drivers x 17 circuits sit at `0x7adb50`
  (`0x7adb4c + 4 + driver*0xf28 + circuit*0xe4`), each a 4-byte header plus four
  `0x38`-byte sub-records. Lap records are 17 records of `0xe8` bytes at `0x7ab760`, merged
  from `RECORDS.REC` by `0x54fa80`; the time formatter `0x52ad47` (through `0x52ad57`)
  skips times of `0x3e80000` ms (65,536 s, about 1,092 minutes) or more.
- **Clocks and pit stops.** A free-running millisecond race clock at `0x66d814` is
  compared with per-car deadlines. `0x432acb` steps the pit-stop timer and `0x4364b6`
  checks pit-stop timing; `0x43c4c1` converts the clock to rounded minutes.
- **Text and menus.** All UI text goes through one pointer table `[0x835846 + 4*id]`,
  zero in the file and filled by the text loaders; the highest id any caller computes is
  `0xc11` (3,090 ids). The interpreter `0x54d798` expands entries: byte 0 ends, byte 4
  escapes to a handler table at `0x54d7ac`, other bytes below `0x14` copy code runs, and
  bytes from `0x14` up are literals. Menus are static `.data` structures: a header plus
  chained items carrying a rectangle, id, value type and cell, text id, and draw and input
  handlers. The current menu is `[0x82fd34]`.
- **Sound.** 16 channels (arrays from `0x88f708`, active mask `0x88f704`) and 100 sample
  records of `0x1c` bytes at `0xb1c8e8`; the master level `0x88fba4` is clamped to
  `0x4000`. `0x51a78d` drives sound each frame and `0x519bb2` triggers engine sound.
  DirectSound is reached through a chain of objects from `[0xb1d3d8]` to a device wrapper
  whose `+0x1a4` holds the `IDirectSound` pointer.
- **Input.** A 256-bit key-state bitmap at `0x8245a0` and a 64-word key queue at
  `0x8245c8`. The key-binding table sits inside `.text` at `0x52de19` (lookup `0x52dbc2`).
  Eight `0x34`-byte controller presets at `0x7ab5b4` (keyboard, joypad, joystick, wheel and
  four user slots); six DirectInput device slots at `0xae6b68` are filled by the
  `EnumDevices` callback `0x5078b0`; force-feedback effects go through `0x5069d0`
  (currently excluded from the queue as suspected runtime code).
- **Replay and network.** One opcode stream (buffer `0x62b19a`, cursor `0x62d1a0`)
  appears to serve both; `0x4450e5` writes 10-byte replay records, `0x4453d1` sends and
  `0x445423` receives, through the packet send wrapper `0x507410` and the DirectPlay
  receive `0x507360`. The multiplayer front end keeps 22 player slots of `0x1c` bytes at
  `0xb1b8d8`; `0x53fd63`/`0x53fdcc` find slots whose `QueryPerformanceCounter` stamp is
  older than `0x4446390` ticks.
- **Saves and services.** A central service dispatcher at `0x531f9c` (jump table
  `0x531d74`) handles commands such as `0x42` (the `f1gstate.tmp` writer), `0x4b`/`0x4c`
  (packet send/receive) and `0x4f`/`0x50` (cached temp-file read/write,
  `0x538440`/`0x53832b`). `0x512c7d` copies the run-time state block
  `0x7ad890`-`0x7c3ae4` (`0x16254` bytes) into the saved-state buffer; replay events pass
  through the temp-file cache 8 bytes at a time.

</details>

Named functions so far are listed in [progress/symbols.txt](progress/symbols.txt).

## Layout

| Path | Contents |
|---|---|
| [src/gp4/asm/](src/gp4/asm/) | accepted reconstructions of functions the analysis classified as hand-written assembly (`0040`-`0044` core, `0051`-`0055` game logic) |
| [src/gp4/c/](src/gp4/c/) | accepted reconstructions of functions classified as compiled code (most still reconstructed with the register convention) |
| [src/gp4/runtime/](src/gp4/runtime/) | registry bounds, `gp4_call_regs` (calls original register-convention routines), freestanding CRT shims |
| [include/gp4/](include/gp4/) | `gp4.h` conventions, `x87.h` bit-exact x87 primitives, `flags.h` x86 flag results |
| [specs/layouts/](specs/layouts/) | 1,529 emulator input layouts ([format](specs/layouts/README.md)) |
| [specs/](specs/) | evidenced exemptions: constant ranges, unreachable blocks, stack-dependent flags, fixed-work routines, shape |
| [progress/](progress/) | committed snapshots: `ledger.txt`, `report.json`, `regions.json`, `symbols.txt` |
| [hook/src/](hook/src/) | in-game harness, a `dinput8.dll` proxy |
| [gp4re/](gp4re/) | the Python pipeline (`python -m gp4re ...`) |
| [config/project.toml](config/project.toml) | single settings file: pinned executable, oracle settings, queue |
| [tests/](tests/) | Python soundness and pipeline tests, native x86 tests |
| [tools/](tools/) | lead helpers (switch repair, flag liveness, status block, unattended runs) |
| [prompts/](prompts/), [.claude/](.claude/) | protocols and agent definitions for the lead and workers |
| [docs/](docs/) | strategy, research survey, verification bar, checkpoints |

Both `src/gp4/` tracks are split into folders named by the first four hex digits of the
address.

Kept out of git (`.gitignore`): your game install (`Grand-Prix-4/` or `input/`);
everything that holds original bytes or dumps - `analysis/`, `state/` (the ledger
database), `build/`, `runtime/` (the game copy for in-game tests), `logs/`, `*.lst`; and
locally downloaded tools in `toolchain/`.

## Toolchain

| | This project |
|---|---|
| C/C++ compiler | a modern MSVC targeting x86 with x87 floating point: `/O2 /Oy- /arch:IA32 /fp:precise /GS- /std:c++17` |
| Assembler | none: assembly routines are reconstructed as C++ with `gp4::x87::*` primitives |
| Linker | reconstructions link into a fixed-base DLL without a CRT for the oracle, and into the hook DLL (`/MT`) |

The original tools are listed under [How the game is built](#how-the-game-is-built).

- **Python 3.11+** with `pefile`, `capstone` and `unicorn` >= 2.1 ([requirements.txt](requirements.txt)).
  Reconstructions match Unicorn 2.1.4's model of undefined flags; use that version.
- **Visual Studio or Build Tools** with the C++ x86/x64 toolset
  (`Microsoft.VisualStudio.Component.VC.Tools.x86.x64`), found with `vswhere`; or set
  `GP4RE_VCVARS` to a `vcvars32.bat`.
- **Ghidra** (optional) only adds pseudocode to the context a contributor sees; no
  acceptance check uses it. Run once:

  ```bash
  python -m gp4re ghidra analyze
  ```

  then:

  ```bash
  python -m gp4re ghidra export
  ```
- **Visual C++ 6** (optional, future): a placeholder for byte matching (T3) is reserved
  in `config/project.toml` (`[compiler.msvc6]`); nothing uses it yet.

## Quick start

Bring your own `GP4.exe` - the build with the SHA-256 above. Put your Grand Prix 4 install
in `Grand-Prix-4/` or `input/`, or set `GP4RE_GAME_DIR` to its folder (`GP4RE_EXE` is
enough for analysis, but `runtime install` needs the install folder). Nothing in that
folder is ever modified.

```bash
python -m pip install -r requirements.txt
```

Fingerprint the executable, recover functions, map the regions and create a local ledger
(`state/gp4re.sqlite`):

```bash
python -m gp4re analyze
```

A fresh ledger starts with every game function to do: the progress figures above come
from the maintainer's ledger, which is not distributed with the repository. A fresh
analysis also lacks the maintainer's later repairs (`tools/fix_switches.py`,
`tools/add_starts.py`) and exclusions, so it finds about 12,838 functions rather than
12,931; rerun the repair tools after `analyze` if you need the same function list.

Check Python packages, the executable, the MSVC x86 toolchain and the analysis:

```bash
python -m gp4re doctor
```

Show progress:

```bash
python -m gp4re status
```

Re-check one reconstruction against the original. `verify` runs every gate on the file in
`src/`, whatever the ledger says, so it works on a fresh clone:

```bash
python -m gp4re verify 0x00401d80
```

It links every reconstruction in `src/` into one DLL first; compiled objects are cached
in `build/obj`, so only the first run compiles everything.

## Verifying a function

Each reconstruction lives in its own file, `src/gp4/<track>/<first 4 hex digits>/<address>.cpp`.
Only `python -m gp4re work accept` writes there, and only after lint, build and shape
pass and the emu gate passes. If emulation is blocked by a Windows API the oracle does not
model, a function whose shape matches closely is accepted as `accepted-runtime`, pending
the in-game check (T2).

| Gate | Checks |
|---|---|
| lint | exactly one `// FUNCTION: GP4 0x...` annotation and one `GP4_IMPL`; no inline assembly, `volatile`, codegen pragmas or system headers beyond `stdint`/`stddef`/`string` |
| build | compiles and links with every other accepted function, x87 floating point |
| shape | calls exactly the same GP4 functions and Windows APIs as the original; its global and string references must largely match (Jaccard similarity 0.6 or more, below which the gate fails); lead-proven dead calls and reads are exempt ([specs/shape_exempt.json](specs/shape_exempt.json)) |
| emu | original and reconstruction run in one Unicorn address space on the same generated inputs: by default 200 random cases, then up to 1,500 coverage-guided cases (stopping once coverage is met or after 25 s), covering at least 90% of the original's basic blocks and 70% of its branch directions; 4 lead-proven fixed-work routines run 2-3 cases ([specs/heavy.json](specs/heavy.json)), and lead-proven dead code is left out of the coverage requirement ([specs/coverage_exempt.json](specs/coverage_exempt.json)) |

Matching is **bit-exact** by default: return value, every declared output register, x87
value and flag, and every caller-visible memory byte. A spec may set a floating-point
tolerance (`tol=`) openly, but no accepted reconstruction uses one. The emulator runs at
control word `0x037F` and runs GP4's own initialisers (`0x527453`, `0x527476` and the
CRT's `_cfltcvt_init` at `0x5da8f7`) before any test. What T1 does and does not prove is
set out in [docs/STRATEGY.md](docs/STRATEGY.md) section 3 (Acceptance hardening).

| Tier | Oracle | Proves |
|---|---|---|
| T0 | build + shape audit | it compiles and touches what the original touches |
| T1 | differential emulation ([gp4re/emu.py](gp4re/emu.py), Unicorn) | same outputs and memory effects on generated inputs |
| T2 | in-game hooks ([hook/](hook/)) | the game still runs identically with the reconstruction swapped in |
| T3 | byte matching with your own Visual C++ 6 (optional, not implemented) | identical machine code |

| State | Meaning |
|---|---|
| `accepted` | passed every gate including full T1 emulation |
| `accepted-runtime` | passed lint, build and shape, but emulation stops at a Windows API the oracle does not model; must be confirmed in game (T2) |
| `deferred` | could not be passed yet; the blocker and what it needs are recorded |
| `todo` | queued |
| `excluded` | library, compiler or data code; not reconstructed |

Each file carries a reccmp-style `// FUNCTION: GP4 0x...` annotation. Byte matching is
not a goal. For the compiled-C functions it could be added later if someone supplies
Visual C++ 6 (T3); it cannot apply to the hand-written assembly core.

Calls to functions not yet reconstructed go to their original addresses, so every
reconstruction runs against the real binary from the start. An example from the core,
[src/gp4/asm/0040/00401d80.cpp](src/gp4/asm/0040/00401d80.cpp):

```cpp
#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00401d80
// Takes a pointer in EDX to a block holding three angles as doubles at +0x10,
// +0x18, +0x20 and stores (sin, cos) pairs for each at +0x28/+0x30, +0x38/+0x40,
// +0x48/+0x50. Building block of the rotation-matrix setup that follows it.
void __cdecl Orient_ComputeAngleSinCos(gp4::Regs* r) {
    uint8_t* o = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));
    for (int i = 0; i < 3; ++i) {
        double s, c;
        gp4::x87::fsincos(GP4_FIELD(double, o, 0x10 + 8 * i), &s, &c);
        GP4_FIELD(double, o, 0x28 + 16 * i) = s;
        GP4_FIELD(double, o, 0x30 + 16 * i) = c;
    }
}
GP4_IMPL(0x00401d80, Orient_ComputeAngleSinCos, "regs(edx:ptr[0x60]:f64) -> ()")
```

The last string is the function's interface and test-input spec: here a register-convention
routine whose EDX points at `0x60` bytes of doubles, with no register outputs (memory
writes are always compared). The grammar is in [gp4re/spec.py](gp4re/spec.py).

## In-game testing

[hook/src/](hook/src/) builds a `dinput8.dll` proxy that `jmp`-patches accepted
reconstructions into a **copy** of the game, toggled per function in `gp4hook.ini`. It can
also record which functions run in each phase of a session (F9 writes a phase marker) and
sample chosen values every simulation tick to `telemetry.csv`.

```bash
python -m gp4re runtime install
```

This copies your install to `runtime/GP4/`, builds the hook with every accepted
reconstruction, and enables every function of 5 bytes or more that the ledger lists as
accepted or accepted-runtime. You then launch the copy yourself. Results land in
`runtime/GP4/gp4hook.log`; `coverage.log` only if `GP4RE_COVERAGE=1` was set before
install (press F9 to mark phases); `telemetry.csv` only after setting
`[telemetry] callsite` and `[watch]` in `gp4hook.ini`.

**Status: not yet run in game.** The hook DLL builds and its register adapter passes the
native tests, but no in-game session has been recorded. The 149 accepted-runtime functions
still await T2, and the runtime-coverage (M2) and telemetry-baseline (M3) milestones are
open.

## Contributing

The work loop is the same by hand or by agent: claim a function, read its context, write
a candidate, and let the verifier decide.

```bash
python -m gp4re work claim --count 1 --max-size 48 --context
```

The claim prints the address you were given; use it for `<addr>` below (scratch folders
use the 8-digit form, e.g. `build/scratch/00401d80/`). Read its context packet again at
any time:

```bash
python -m gp4re work show <addr>
```

Write the candidate to `build/scratch/<addr>/v1.cpp`, then:

```bash
python -m gp4re work try <addr> build/scratch/<addr>/v1.cpp
```

```bash
python -m gp4re work accept <addr> build/scratch/<addr>/v1.cpp
```

If it cannot pass, record why instead of forcing it:

```bash
python -m gp4re work defer <addr> "why it cannot pass yet" --needs "what would unblock it"
```

Claims are atomic and leased for two hours within one checkout and its git worktrees.
The ledger is not distributed with the repository, so a fresh clone does not know which
functions are already done: check [progress/ledger.txt](progress/ledger.txt) before
starting on an address, and leave anything listed there as accepted alone.

Most of the work so far was done by a lead session coordinating worker sub-agents in
Claude Code; the rules every contributor follows are in [AGENTS.md](AGENTS.md), and the
lead, worker and layout protocols are in [prompts/](prompts/). Names need evidence
(strings, callers, behaviour, community references); without it, keep `FUN_`/`DAT_`.

- How and why it is built this way: [docs/STRATEGY.md](docs/STRATEGY.md)
- What T1 does and does not prove: [docs/STRATEGY.md](docs/STRATEGY.md) section 3
- Prior art and related projects: [docs/RESEARCH.md](docs/RESEARCH.md)
- Latest checkpoint: [docs/CHECKPOINT-2026-10-20.md](docs/CHECKPOINT-2026-10-20.md)

### Maintainers: before committing

These steps need the maintainer's ledger (`state/gp4re.sqlite`).

Run the test suite:

```bash
python -m unittest discover -s tests -v
```

Re-run every gate on every accepted function (100 random cases by default; pass
`--cases 200` to match acceptance):

```bash
python -m gp4re regress
```

Then refresh the committed snapshots (lead only) and the status block above:

```bash
python -m gp4re export
```

```bash
python tools/readme_status.py
```

## License

This is an unofficial project for study, preservation and interoperability. It is not
affiliated with or endorsed by Geoff Crammond, Simergy, the game's publisher or the
Formula One rights holders. Grand Prix 4, its code, data and trademarks belong to their
respective owners.

The project's own work (tooling, headers, harness and the source written here) is
released under the MIT License ([LICENSE](LICENSE)). The licence covers that work only and
grants no rights in Grand Prix 4. No original game files are distributed (see the top of
this page); you need your own copy of the game to build or verify anything.

Community work used as a reference: GP4PP / GP4MemLib by Oggo87 (MIT). Its patch site
confirmed the build, and its addresses were used as leads. Its `CarDynamicData` layout
(from the work of Rene "SDI" Smit and Paulo "Prblanco" Blanco) has not yet been compared
with the car records described above.
