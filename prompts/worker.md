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
  Calling a different function than the original does is a wrong callee (shape gate).
- Globals: `GP4_GLOBAL(float, 0x006a1234)`, arrays `GP4_ARRAY(int16_t, 0x...)`.
  A string the original references: `GP4_ARRAY(const char, 0x...)` or the identical literal.
- Win32: `GP4_IAT(fn_type, <slot from the packet>)(args)` - no `<windows.h>`;
  add `; emu=skip:win32 <Api>` to the spec (accepted pending an in-game check).
- One-off field offsets: `GP4_FIELD(float, p, 0x2c)`. When a layout is clear,
  declare a struct instead (fields named from evidence, else `UnknownXX`).

## x87 fidelity (the oracle compares bits)

- Use `gp4::x87::fsin/fcos/fsincos/fsqrt/fpatan/fptan/frndint/fscale/fprem`
  wherever the original executes that instruction - never `<math.h>`.
- `fistp` rounds with the current mode: `gp4::x87::fistp32(x)`, not a C cast.
- Match load/store widths: `fld dword` is float, `fld qword` is double. An
  intermediate the original stores to a float slot: `gp4::x87::to_f32(x)`.
- Keep the original operation order (`a*b + c` vs `c + a*b` can differ).
- x87 inputs/outputs must be contiguous from `st0`. Output capture checks raw
  80-bit x87 values before rounding. If an output cannot survive the `Regs`
  double slot exactly, the oracle reports INCONCLUSIVE; defer for an extended
  precision adapter rather than relaxing tolerance.

## Spec checklist (the emulator needs plausible inputs)

- Pointer sizes: largest displacement used + 8. Fill floats where the code reads
  floats (`ptr[0x60]:f64`); random bytes become NaNs and the original's cases go
  invalid. A pointer inside a pointed-to struct: `:layout=<name>` if the lead
  provided one, else ask in `needs`.
- Indices into GP4 tables: ranges (`i32[0..21]`), or the original faults.
- `tol=` only after the bit-exact attempt shows tiny float noise you cannot remove.
- "INCONCLUSIVE ... external" means the original calls Win32: use `emu=skip:win32 <Api>`.

- Every generated original case must be valid for full acceptance. Basic block
  and branch coverage are enforced, with bounded extra input search. Missing
  GPR, arithmetic-flag and stack inputs are checked by changing the original's
  undeclared inputs. Report the missing input or layout when these gates fail.
- Lead-provided global field layouts use `; globals=layout=<name>`. They vary
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
- For a single 32-bit scalar global getter/setter that does not dereference it
  or use it as an index/divisor, use `globals=fuzz:i32` without another lead
  request. Byte/word fields, mixed-width state and bounded tables need layouts.
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
