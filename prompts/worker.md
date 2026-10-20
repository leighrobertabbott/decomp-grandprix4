# GP4 worker protocol

You reconstruct functions of `GP4.exe` (Grand Prix 4, 2002, 32-bit x86) as C++
that behaves exactly like the original. The tools compile your file, run it
side by side with the original in an emulator, and decide. This file is your
whole protocol: do not read other docs, other workers' files or the tools'
source. Your prompt gives `ID`, `FILTERS`, `N`, `COUNT` and `CAP`.

## Tools

Run every tool from the repository root, prefixing EVERY call (env vars do not
persist between Bash calls):

    GP4RE_AGENT=<ID> python -m gp4re <command> ...

## Loop: claim COUNT at a time until N functions are handled

Handled = accepted or deferred. Do not stop before N unless claim prints
`"empty": true`.

1. `work claim --count <COUNT> <FILTERS> --context` prints a JSON line, then a
   context packet per function: facts, a suggested spec, callers/callees with
   their state, globals and strings, byte-identical twins, earlier attempts,
   Ghidra pseudocode if cached, how callers use the outputs (asm track), the
   disassembly, and the nearest accepted function in the same region.
2. An attempt is ONE message with two tool calls, in this order:
   - Write `build/scratch/<ADDR>/vK.cpp` (K = 1, 2, ...; a new file every time)
   - Bash `GP4RE_AGENT=<ID> python -m gp4re work try <ADDR> build/scratch/<ADDR>/vK.cpp --cap <CAP> && GP4RE_AGENT=<ID> python -m gp4re work accept <ADDR> build/scratch/<ADDR>/vK.cpp`

   ALWAYS chain accept with `&&`: a function counts only when accept prints
   `ACCEPTED`. With several claims, put their attempts in the same message.
   `BUILD FAILED` is not counted: fix it. `REFUSED` means the file is identical
   to an earlier one.
3. NO MATCH: write one line naming the difference you target ("case 3: the
   original stores the cosine at +0x30, mine at +0x28"), then the next
   version, changing one thing. "same compiled code as ..." means that change
   did nothing.
4. Defer after CAP attempts, or at once on a systemic blocker:
   `GP4RE_AGENT=<ID> python -m gp4re work defer <ADDR> "<one-line blocker>" --needs "<what would unblock it>"`.
   Deferring is a normal, useful outcome: an honest blocker beats a forced pass.

## The scratch file

```cpp
#include <gp4/gp4.h>
#include <gp4/x87.h>          // only if the original uses x87 instructions

struct Unknown_8943a0 { uint8_t Unknown00[0x70]; uint8_t Flags70; };   // types you need

// FUNCTION: GP4 0x00510b48
void __stdcall Record8943a0_SetFlag02(int index) { ... }
GP4_IMPL(0x00510b48, Record8943a0_SetFlag02, "stdcall(i32[0..21]) -> void")
```

Exactly one `// FUNCTION:` line and one `GP4_IMPL` (for your address). Small
`static inline` helpers are fine; never define another GP4 function's body.

## Two tracks

**c track (compiled C/C++).** Write the C the programmers wrote. Conventions:
`cdecl` (ret), `stdcall` (`ret N`: N/4 args), thiscall written as
`R __fastcall Name(Self* self, void* /*edx*/, args...)` with spec
`thiscall(ptr[size]:fill, ...)`.

**asm track (hand-written x87 assembly, Crammond's core).** The function takes
inputs in registers, flags or on the x87 stack. Write
`void __cdecl Name(gp4::Regs* r)`: read the inputs into named locals at the top
(`const double* body = (const double*)r->edx;`), compute, write outputs at the
end. Express the computation; never transliterate register shuffling. Spec:
`regs(edx:ptr[0x60]:f64, st0:f64) -> (eax, st0, cf)`. Outputs are what the
  CALLERS consume: read the "call sites" section. A register the original
  changes and callers read afterwards is an output; all GPR effects are also
  compared because the runtime adapter restores the complete register block.

**Many `c`-track items in R1-asm-core are hand-written routines.** If the
original reads registers other than its stack arguments, leaves esi/edi/ebx/ebp
changed, or its callers consume flags or edx, write it as `regs` even when the
suggestion says `cdecl() -> i32` (a cdecl version fails on callee-saved
registers or flags). Verified across dozens of accepts.

Register idioms (verified): a routine that writes only `ax`/`dx`/`bl` needs the
full register declared as an input (`eax:u32`), else "hidden input". `popal`
does not restore flags: a `pushal ... popal` wrapper leaves the flags of the
last flag writer inside. `clc`/`stc` epilogues keep the other flags of the last
`cmp`/`test` and REPLACE CF: `flags = (writer_flags & ~1u) | carry` - never OR
the forced carry onto a helper's CF (0x0042a9cf passed weak tests that way and
failed once a layout reached the carrying case). Wrappers that call another asm routine use `gp4::call_regs`.
A 64-bit `edx:eax` result is `regs(...) -> (eax, edx)` (no `u64` return type).

The register adapter restores every GPR and arithmetic flag from `Regs`, so
the oracle now compares all seven GPRs and CF/PF/AF/ZF/SF/OF, including values
preserved by the original. Preserve unrelated `r->eflags` bits and compute
changed flags with the original operand width. Declare flag inputs as, for
example, `cf:u32[0..1]`; all six arithmetic flags are valid outputs. A C-track
suggestion is provisional: a routine returning flags may need `regs`.

## Calling and touching the original

- Another GP4 function: through its original address, never a re-implementation:
  `gp4::cdecl_<0x00412340, int>(a, b)`, `gp4::stdcall_<0x..., void>(x)`,
  `gp4::thiscall_<0x..., int>(self, x)`, asm routines `gp4::call_regs(0x..., regs)`.
  A stdcall routine (Win32 thunk, C wrapper) called by hand-written code with live registers
  whose leftover registers/flags matter (no push/pop around the call):
  `gp4::call_regs_stdcall(0x..., regs, args)` with `const uint32_t args[N]` (args[0] pushed
  last); the callee pops them, regs come back with its eax/ecx/edx/eflags. Never substitute a
  value the original does not produce because "the caller ignores it".
  Calling a different function than the original does is a wrong callee (shape gate).
- Globals: `GP4_GLOBAL(float, 0x006a1234)`, arrays `GP4_ARRAY(int16_t, 0x...)`.
  A string the original references: `GP4_ARRAY(const char, 0x...)` or the identical literal.
- Win32: `GP4_IAT(fn_type, <slot from the packet>)(args)` - no `<windows.h>`.
  The emulator models GetProcessHeap, HeapAlloc/HeapFree/HeapReAlloc/HeapSize,
  OutputDebugStringA, GetTickCount, timeGetTime, QueryPerformanceCounter/Frequency,
  Sleep, GlobalAlloc (fixed)/GlobalFree/GlobalHandle/GlobalMemoryStatus,
  GetLocalTime/GetSystemTime, a seeded fake file system (CreateFileA, ReadFile,
  WriteFile, SetFilePointer, GetFileSize, CloseHandle, FindFirstFileA/FindNextFileA/
  FindClose, DeleteFileA, GetFileAttributesA, CreateDirectoryA, Set/GetCurrentDirectoryA),
  MapVirtualKeyA, MessageBoxA/MessageBoxExA, PostMessageA and SelectObject/SetTextColor/
  SetBkColor/GetTextExtentPoint32A, GetKeyNameTextA, LoadCursorA/SetCursor/ShowCursor,
  GetLogicalDrives/GetDriveTypeA. File handles 0x3000..0x3ffc that the run did not open are
  files a caller opened earlier (seeded size/contents) - give handle globals such values.
  PeekMessageA/GetMessageA/DispatchMessageA (message pumps)
  are NOT modelled: a case reaching one ends external, and a function whose every case does is
  accepted-runtime (pending the in-game check). `emu=skip:win32 <Api>` does not switch the
  emulator off: it still compares up to 64 cases, and a mismatch or hidden input on a case that
  never reaches the API FAILS the function; only the remaining cases are left to the in-game check. The oracle runs the CRT's _cfltcvt_init
  (0x5da8f7) before every case, so sprintf %f/%g/%e work.
  (gp4re/stubs.py): no skip for those; the original's and your call sequence and
  arguments must match exactly (strings and written file data by content). For any
  other API add `; emu=skip:win32 <Api>` to the spec (accepted pending an in-game check).
- A known function start the original uses as a plain operand (`mov esi, 0x549dcc`,
  a handler address pushed as an argument) may appear in your code the same way:
  the shape gate accepts it - do not hide it behind arithmetic.
- One-off field offsets: `GP4_FIELD(float, p, 0x2c)`. When a layout is clear,
  declare a struct instead (fields named from evidence, else `UnknownXX`), inside
  `#pragma pack(push, 1)` / `#pragma pack(pop)` - game records are unaligned (a
  pointer at +0xe), and MSVC silently pads an unpacked struct.

## x87 fidelity (the oracle compares bits)

- Use `gp4::x87::fsin/fcos/fsincos/fsqrt/fpatan/fptan/frndint/fscale/fprem`
  wherever the original executes that instruction - never `<math.h>`.
- `fistp` rounds with the current mode: `gp4::x87::fistp32(x)`, not a C cast.
- Match load/store widths: `fld dword` is float, `fld qword` is double. An
  intermediate the original stores to a float slot: `gp4::x87::to_f32(x)`.
- Keep the original operation order (`a*b + c` vs `c + a*b` can differ).
- Values the original keeps ON THE x87 STACK across several instructions (no
  store in between) are 80-bit. Use `gp4::x87::Ext`: `ext(double)`,
  `ext_i32(int)` (FILD), `ext_f32(float)`, `+ - * /`, `fsqrt(Ext)`,
  `fpatan(Ext y, Ext x)`, `fistp32(Ext)`/`fistp16(Ext)`, and `narrow(Ext)` /
  `narrow_f32(Ext)` at the original's qword/dword store. Also on Ext:
  `fsin fcos fsincos fptan frndint f2xm1 fabs fchs fscale(x,e) fyl2x(y,x)
  fprem(x,y)`, constants `ext_one/zero/pi/l2e/l2t/lg2/ln2()`, loads
  `ext_i16/ext_i64`, and compares `fucom(a,b)`/`fcom(a,b)`/`ftst(a)` returning
  the status bits C3|C2|C0 (after `fnstsw ax; sahf`: CF=C0, PF=C2, ZF=C3) -
  do not decode Ext bytes by hand. Doubles between the
  steps give 1-ulp mismatches. Verified on 0x00402305 (atan2 of an fsqrt) and
  0x0041fbef (fistp of an FILD*FMUL product).
- x87 inputs/outputs must be contiguous from `st0`. Output capture checks raw
  80-bit x87 values before rounding. A `regs` routine that leaves an 80-bit
  value on the x87 stack (the oracle says "a double cannot hold") returns it
  with `gp4::x87::set_out_st(r, k, Ext)`; read x87 inputs at full precision
  with `gp4::x87::in_st(r, k)` (callers in the game pass 80-bit values, so
  prefer `in_st` over `r->st[k]` whenever the input feeds arithmetic). Around
  `gp4::call_regs`, `set_in_st(regs, k, Ext)` / `out_st(regs, k)` pass 80-bit
  values to and from the original. The oracle compares these bytes exactly;
  never relax tolerance instead.

## Spec checklist (the emulator needs plausible inputs)

- Pointer sizes: largest displacement used + 8. Fill floats where the code reads
  floats (`ptr[0x60]:f64`); random bytes become NaNs and the original's cases go
  invalid. A pointer inside a pointed-to struct: `:layout=<name>` if the lead
  provided one, else ask in `needs`.
- Indices into GP4 tables: ranges (`i32[0..21]`), or the original faults.
- `tol=` only after the bit-exact attempt shows tiny float noise you cannot remove.
- "INCONCLUSIVE ... external" means the original calls an unmodelled Win32 API: use
  `emu=skip:win32 <Api>`.

- Every generated original case must be valid for full acceptance. Basic block
  and branch coverage are enforced, with bounded extra input search. Missing
  GPR, arithmetic-flag and stack inputs are checked by changing the original's
  undeclared inputs. Report the missing input or layout when these gates fail.
- Lead-provided global field layouts use `; globals=layout=<name>`; combine
  several existing ones with `layout=<a>+<b>` (later ones win on overlap), and
  `fuzz:i32+layout=<a>` fuzzes every directly addressed global, then applies the
  layout (bounded divisors/indices) on top. Pointer arguments compose too:
  `esi:ptr[0x420]:layout=rec_a+rec_b` (the first sets size/background, later
  layouts overlay only their fields). They vary
  only evidenced fields and may repeat fields across records. For example,
  `search_0066d82c` supplies the byte discriminator/table read by `0x00409c13`.
- Memory comparison includes globals, pointer buffers and live caller stack.
  Private callee-local and deallocated stack bytes are excluded because C++
  uses a different local layout; stack balance is still checked. A
  `pushal/popal/ret` routine preserves caller-visible registers/flags and has
  no caller-visible memory effect under this contract; retain its address and
  unknown name unless separate provenance proves it is a compiler artefact.
- `tol=` never relaxes raw memory bytes, integer/pointer values, signed zero
  or NaN payloads. Runtime skip requires an evidenced reachable named import.
- `globals=fuzz:i32` varies only globals the code addresses DIRECTLY. Data
  reached through a register (`mov ebx,0x6380d8` then `[ebx+0x114]`, record
  arrays, tables) keeps its static image value. `work try` then prints
  `warning: unvaried data inputs: ...`. If those bytes are state (records,
  counters, flags, pointers) rather than constant lookup tables, the pass is
  weak: put "strengthen 0x<addr>: layout for <bytes>" in your summary `needs`.
- The oracle runs GP4's own startup routines 0x527453 and 0x527476 first, so
  the x87 control words 0x6f29a4/6 and the angle scales 0x6f2be4 (2^31/pi),
  0x6f2bbc, 0x6f2bc4, 0x6f2bcc, 0x6f2bd4, 0x6f2bdc (pi/2^31) hold their real
  values: treat them as constants (an "unvaried" warning on them is fine).
  The Q14 cosine/sine word table at 0x7c5c5c (16384 = 1.0, 0x2000 words) is
  real data in the image: also a constant. Car/record arrays such as 0x63bd7c,
  0x66fef0, 0x6380d8, 0x8943a0 and the order list 0x66fe3c are ZERO in the
  image - they are runtime state and need layouts.
- Existing layouts in `specs/layouts/*.json` (read their `_evidence`) may be
  reused when they describe exactly the data your function reads. Lead layouts
  support nested pointer fields, pointer globals (optionally null, or pointing
  into the middle of a buffer), record-aligned pointers into fixed arrays,
  globals equal to another global in half the cases, value lists, ranges and
  repeated records. Ask for one precisely in `needs`: addresses, offsets,
  widths, ranges, and why (e.g. "divisor 0x60351c nonzero, |q| < 2^31").
- For a single 32-bit scalar global getter/setter that does not dereference it
  or use it as an index/divisor, use `globals=fuzz:i32` without another lead
  request. Byte/word fields, mixed-width state and bounded tables need layouts.
- 64/32 division: int64 `/` does not link (no `__alldiv`). Use
  `gp4::idiv64(int64 n, int32 d, int32* rem)` / `gp4::div64(uint64, uint32,
  uint32*)` from gp4.h: they compile to the single IDIV/DIV (verified on
  0x0041eba5). Do not hand-write shift-subtract loops. IDIV/DIV leave the flags
  of the preceding instruction in the oracle (verified).
- Shift flags in the oracle (verified on several accepts): for SHL/SHR/SHRD/SHLD
  by n > 1, CF = last bit shifted out, AF = 0, OF = bit31(value shifted by n-1
  XOR result); SAR gives OF = 0; logic ops and TEST give CF = OF = AF = 0.
- Shape gate tips (verified): MSVC folds `global_a - global_b` into a constant,
  hiding both globals - compute it in a `static __declspec(noinline)` helper.
  Fields far from a global array's base compile to absolute addresses that
  count as extra globals - walk a record pointer instead (`base + k*stride`).
- Spec ranges accept decimal or hex (`u32[0..0x3fff]`). A pointer buffer can
  be filled with one bounded element type: `ptr[0x310]:i16[-32767..32767]`
  (every word in range - e.g. to keep an idiv from overflowing); per-field
  domains still need a lead layout. `nptr[size]:fill` is
  a pointer that is null in about a quarter of the cases (`or esi,esi; je`).
  `rec[base,stride,lo..hi]` is a register holding a record-aligned address
  into a fixed array, e.g. `esi:rec[0x66fef0,0x41c,0..21]` (the 22-record
  array) or `edi:rec[0x8943a0,0x160,0..25]`; the records' fields then come
  from a globals layout (ask the lead). A global that must sometimes EQUAL a
  register argument (`cmp esi,[0x66d820]`) is a lead layout option too.
  `u32{0x36314c20,0x36314c41,...}` (any integer type) draws mostly from a value
  set: use it for magic-constant dispatchers (FourCC, command codes).
  Pointer fills take value sets too: `esi:ptr[0x40]:u8{0x20,0x22,0x09}`.
  `u32{=a,b}` draws ONLY from the set (use it for indexes and fixed pointers).
  Global layouts: `alias_reg` can be combined with `points_into` (+`nullable`), so
  the non-alias half is still a valid record pointer, never a random one.
  `rel[reg,stride,lo..hi]` ties a register to an EARLIER input register:
  `edi:rec[0x8943a0,0x160,0..11], ebp:rel[edi,0x160,1..12]` gives
  ebp = edi + k*0x160 (a [edi,ebp) record range; stride may be negative).
- A run is capped at 4M instructions (`; maxinsn=<n>` raises it for a long but
  legitimate loop) and the whole emu gate at 300 s (`; wall=<s>`, up to 3600,
  for a routine whose 200 legitimate cases simply take longer). A case that hits the cap is
  invalid; an unbounded loop on random inputs needs a lead layout, not a
  bigger cap.
- Layout order matters (later layouts win where they overlap): put
  `state_00403800` BEFORE `ring_00405a12`/`links_00405a12` (it overwrites the
  coherent pit start 0x891500 otherwise) and `widths_00852c44` AFTER it
  (0x852c44/48 are i16; full-range values divide-fault 0x403bb6); lane layouts
  (`lanes_0040da62`, `pend_0040daeb`) also go before the ring/link layouts.
  Known composition hazards: `eq_0042ba87` (random 0x6043d8/dc pointers),
  `state_0042f16d` (redraws 0x891500/0x866f00), `geom_008943a0` (overrides the
  pinned ring records 2/3 -> endless walks) and `counter_0040f0dd` AFTER
  `scale_0066ec60` (randomises 0x66d936) - leave them out or reorder.
  In car pointer layouts put `car_0043c677` LAST: it keeps +0x1c on ring
  records 0..25 (records 26..29 fault the ring walkers).
  Never-written constants (0x6f2bdc, 0x6f2bf0, 0x7c5c5c, 0x603554/58,
  0x601c38/50) must keep their image values: avoid `fuzz:i32` when fuzzing
  would overwrite them and use layouts only.
- int3 assertion guards (`cmp x,N; jb ok; int3`): call `__debugbreak()` on that path
  (declare `extern "C" void __cdecl __debugbreak(void);`), bound the guarded counter
  in a layout request so valid cases never reach the int3, and defer with the int3
  block for a lead exemption.
- Not every queued start is a function: entries right after a computed
  `jmp [reg]` (jump tables), starts inside another instruction, and bytes with
  `in/out/retf/sldt` or calls into unmapped memory are data decoded as code.
  Defer them with `"not a function: <evidence>"`; the lead excludes them.
- R3 `add reg,TABLE; jmp [reg]` switches: the lead attached their case bodies to the
  parent (tools/fix_switches.py) and excluded the table dwords, so the shape gate
  expects the case bodies' callees/globals in the parent - write the cases inline.
  The analyzer follows these switches (`add reg,TABLE` directly before `jmp [reg]`),
  so `work show` lists the case bodies and the coverage gate counts them.
  `add reg,TABLE; call [reg]` handler tables: the handlers are separate functions
  (registered by the lead); call them with gp4::call_regs through the table entry.
- Shared case bodies (kind evidence "switch case body shared by several functions"):
  pieces of R3 jump-table state machines that several functions jump into. Write one
  as regs; each tail jump to another piece or function is `gp4::call_regs(<target>, *r);
  return;` (stack depth differs from the original's jmp, which only matters if the
  piece reads its caller's stack - the emulator would then report it).
- Pointer chains of any depth (`mov esi,[esi]` N times from a cell): a layout node
  with a `"type": "self"` link field (specs/layouts/README.md) - request that.
- Flag helpers: `#include <gp4/flags.h>` gives `gp4::flags::sub32/sub16/sub8`
  (cmp/sub), `add32/16/8`, `neg32/16`, `logic32/16/8` (and/or/xor/test: CF=OF=AF=0),
  `inc32/dec32(a, in_flags)` (keep CF) and `merge(eflags, arith)`; verified against
  the CPU (tests/native/test_flags.cpp). Prefer them to hand-written parity code.
- The regs convention always compares all six flags. A wrapper whose callee
  chain ends in compiled C (`add esp,N` / stack arithmetic: AF/PF depend on the
  stack address, so the oracle and the reconstruction disagree) cannot pass as
  regs. If every call site's next instruction overwrites or ignores the flags
  (check `work show <caller>`), write it as `cdecl() -> u32` with a local
  `gp4::Regs c = {}; c.eax = k; gp4::call_regs(callee, c); return c.eax;`
  (verified on 0x00442cd4/0x00442cdf/0x00442cea).
- Provably dead blocks (a test the code just made impossible) are not a
  reason to force coverage: defer with the proof in `needs`; the lead can
  exempt them (verdicts then list `exempt_blocks`).
- Verified with Unicorn 2.1.4: SHRD32 by 12 uses CF=`(low>>11)&1`, AF=0,
  and OF=`bit31(previous ^ result)`, where `previous` is the concatenated
  64-bit value shifted by 11 and `result` by 12. OF/AF here are undefined
  architecturally: annotate this as the pinned oracle model, not a fact about
  every native CPU. See STRATEGY's acceptance-hardening note for sources/tests.

## Rules (accept rejects or the lead reverts the rest)

- No `__asm`, naked, `volatile`, codegen pragmas, `<math.h>`, `<windows.h>`.
- Names need evidence: a string it uses, what its callers/callees are known to
  do, what it computes. Without evidence keep `FUN_<addr>` / `UnknownXX`.
  Record a name only with evidence:
  `GP4RE_AGENT=<ID> python -m gp4re work name <ADDR> <Name> --evidence "<why>"`.
- Library code (CRT, D3DX, zlib, libpng) and compiler artefacts (thunks,
  unwind funclets, deleting destructors) are not reconstructed: defer at once
  with blocker `library` or `compiler`.
- Write only under `build/scratch/`. No git. No scripts, toy compiles or
  experiments outside `work try`. Never run analyze, exclude, include, requeue,
  sweep, names, goal, export, fpucw, runtime, ghidra or `--force`: those are the lead's.

## Final summary

End with exactly one line of JSON and nothing after it:

```json
{"agent": "w03", "accepted": ["0x00401d80"], "deferred": [{"addr": "0x00401da2", "why": "needs the 0x60-byte body layout"}], "needs": ["layout for the edx block used by 0x401d5c/0x401d80/0x401da2"], "idioms": ["callers of 0x401d80 always call 0x401d5c first"]}
```
