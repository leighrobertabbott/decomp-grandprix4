# Reconstruction checkpoint: 2026-10-20

Active goal: `core` (R1, the hand-written x87 simulation core). No STOP file.

## Numbers

| | functions | code bytes |
|---|---|---|
| core accepted | 978 of 1,666 (58.7%) | 80,964 of 259,220 (31.2%) |
| core deferred | 109 (layout authors L03/L04/L06 working through them) | |
| whole game accepted | 979 | 80,992 of 1,730,758 (4.68%) |

The previous checkpoint (2026-10-02) had 33 accepted. Waves e..k used ~150
worker runs (sonnet head/main/asm bands, opus big band) plus six opus layout
authors (L01..L06, `prompts/layout.md`); L01/L02 turned 43 of 49 deferrals into
accepts. 222 evidence-carrying layouts now exist in `specs/layouts/`.

## Verifier and toolchain changes (all with tests; 60+ tests pass)

- **80-bit x87 values**: `gp4::Regs` grew to 0x110 bytes with `st_xin`/`st_xout`
  slots and masks; `gp4::x87::in_st/set_out_st/out_st/set_in_st`. The oracle
  compares 80-bit outputs byte-exactly; the in-game hook and `gp4_call_regs`
  keep both representations. Native adapter test covers it.
- `gp4::x87::Ext` (lossless FLD/FSTP TBYTE) with arithmetic, fsqrt, fpatan,
  fsin/fcos/fsincos/fptan, frndint, f2xm1, fscale, fyl2x, fprem, fabs, fchs,
  load-constant helpers, fild loads, fistp32/16 and fucom/fcom/ftst status bits.
- `gp4::idiv64/div64` (MSVC `_div64/_udiv64` intrinsics = one IDIV/DIV).
- Oracle startup now runs GP4's own `0x527476` after `0x527453`: the six
  angle-scale doubles (0x6f2be4 = 2^31/pi ... 0x6f2bdc = pi/2^31) were zero
  before, so angle code had been tested with zero scales. All accepted
  angle-related functions re-verified with the real constants.
- "Unvaried data inputs" detection: the oracle records game-data bytes the
  original reads that no case varied; `work try` warns, `regress` reports
  weak passes. 100+ weak accepts are listed in `build/weak.txt` for the
  strengthening author L05.
- Input domains: hex ranges; `nptr[]` (nullable pointer args); `rec[base,stride,lo..hi]`
  (record-aligned register values); typed range fills `ptr[n]:i16[a..b]`;
  global layouts with `points_into`, `alias`, `alias_reg`, `values`, ptr
  `bias`/`nullable`; buffer layouts with `count/stride/values/nullable/bias`;
  `globals=layout=a+b` and `globals=fuzz:i32+layout=a` composition.
- Coverage: branch whose target is its fallthrough counts once; lead-proven
  dead blocks/directions in `specs/coverage_exempt.json` (each entry carries a
  static proof; verdicts list `exempt_blocks`/`exempt_branches`).
- Emulator limits: default 4M instructions per run, 300 s per emu gate, and an
  early stop once 8 original cases are invalid (PASS needs all valid).
- Build: link via a response file (the command line had passed 32,767 chars at
  376 objects, which stopped every accept).
- Lead tools: `tools/lead_retry.py` (rerun deferred or strengthen accepted
  candidates under a new spec; claims as lead to avoid races),
  `regress --addr`.

## Corrections made to accepted work

- Six fistp chains that rounded an x87 product to double first now keep the
  80-bit value (0x00402a9a, 0x0040ceb0, 0x00404a33 rewritten; three others were
  already exact).
- An unevidenced local name (`braking` in 0x004143b9) removed.
- False function starts excluded: 0x00407f7f (inside an instruction of
  0x00407ed5), 0x00416674 (gap-filler over a dword table).
- Full regression after the Regs/header changes: every accepted function passed
  (the only failures were a few-minute window with a broken header). A further
  regression under the new startup constants was running at this checkpoint.

## Known limitations (for T2 / later passes)

- 0x00409e29, 0x00409c44, 0x0043d27c, 0x00401000 end with `add esp,0x20`: their returned
  flags depend on the stack pointer; the reconstructions match the oracle's
  stack, not every in-game stack. No caller is known to read them.
- Unicorn's x87 never sets the sticky precision-exception bit (PE): routines that
  copy the status word into eax (`fnstsw ax`, e.g. 0x00402777, 0x00402848) match
  the oracle without PE, while real hardware would show PE. Harmless where eax is
  dead in the callers; a T2 point to check.
- 0x004319dd (popal entry without pushal) needs a stack-frame adapter.
- 0x0040cceb needs a register-to-register pointer relation (ebp = edi + k*0x160).
- ~130 suspicious `imm-ptr` function starts outside core (0x44xxxx..0x51xxxx)
  need an analysis pass before the `legacy` goal; most are probably real
  16-aligned starts inside one mis-sized enclosing function.
- Big-band functions are often weak (record arrays zero in the image); the
  strengthening pass is the way to firm them up before libgp4sim extraction.

## Next

Keep the core bands running (mid asm 300+, mid c 180, big ~90 remain), keep
three layout authors on the deferral queue and L05 on strengthening, then
regress, export, and move to the `legacy` goal once core's queue is drained.
GP4RE_AUTOCOMMIT is unset and the repository still has no commits; nothing was
committed.

## Update (later on 2026-10-20)

| | functions | code bytes |
|---|---|---|
| core accepted | 1,531 of 1,897 (80.7%) | 184,423 of 283,034 (65.2%) |
| core deferred | 45 (layout authors L13/L15/L16/L17 on them) | |
| whole game accepted | 1,532 | 184,451 of 1,730,758 (10.66%) |

- **Region boundary:** R1 now ends at 0x446160 (was 0x440000). The 231
  functions in 0x440000..0x446160 are unaligned and unpadded, reference no
  strings or imports and take register inputs like the rest of R1; aligned,
  nop-padded thiscall C++ starts at 0x446160. Many deferred core functions
  called into this range. The core goal grew from 1,666 to 1,897 functions.
- **Regression** over 1,386 accepted functions: 1,384 passed. 0x004247fa was a
  load-induced false coverage failure (fixed: the phase-2 coverage search
  budget now starts after the base cases). 0x00434e54's count loop needs up to
  ~3.4M instructions, close to the new 4M default cap, so it was re-accepted
  with `maxinsn=8000000`.
- **Verifier and spec changes:** `rel[reg,stride,lo..hi]` register relations;
  global `alias_offset`; `points_into` on buffer-layout fields; global ptr
  fields with `fill: layout=<name>`. Spec strings over 512 bytes were silently
  truncated in the DLL registry reader; they are now read in full, and an
  unterminated string is an error.
- Layout authors L10, L12 and L14 converted 49 of 52 deferrals. L11 strengthened
  55 weak accepts and found no wrong ones; the strengthened domain now
  catches the old 0x40fdfb SF bug. 20 lead-verified dead-code exemptions.
- 580 layouts in `specs/layouts/`.
- Still blocked: orchestrators over long chains of unreconstructed callees
  (0x4097c7, 0x40df1c, 0x40f99e, 0x410abf, 0x40ce15, 0x40df63), which will be
  worked bottom-up; 0x40563c (4096 binary searches per case exceed the 300 s
  emulation cap); 0x43021c (a push-then-jmp entry convention); 0x407ffb
  (never returns); 0x4319dd (popal entry).

## Update (evening): legacy goal R3 under way

| | functions | code bytes |
|---|---|---|
| core (R1) accepted | 1,839 of 1,897 (incl. 4 runtime) | 251,103 of 283,034 (88.7%) |
| core deferred / todo | 41 / 12 | |
| legacy (R3) accepted | 822 (incl. 11 runtime) | 39,118 of ~290,000 |
| whole game accepted | 2,716 | 290,276 of 1,689,279 (17.2%) |

- **Analysis repairs for R3.** R3 dispatches through `add reg,TABLE; jmp|call [reg]`,
  which the analysis does not recognise. `tools/fix_switches.py` excluded 78 queued
  "functions" that were table dwords or case bodies, attached the case bodies of 99
  switches to their parents (shape now expects their callees/globals) and registered
  33 missing handler starts; `tools/add_starts.py` registers individual missed starts.
  About 8 KB of table bytes no longer count as game code. Re-running `gp4re analyze`
  would drop these edits; rerun both tools afterwards.
- **Modelled Win32 imports** (`gp4re/stubs.py`): GetProcessHeap, HeapAlloc/Free/ReAlloc/
  Size, OutputDebugStringA, GetTickCount, timeGetTime, QueryPerformanceCounter/
  Frequency and Sleep run deterministically (bump heap in its own region, seeded clocks,
  identical per case for both runs). Every call is logged with its arguments and the
  original's and reconstruction's logs must be identical; unrepresentable calls stay
  external. Five QPC users (0x51d9ee, 0x51da02, 0x54e615, 0x54e665, 0x54e6af) moved
  from `accepted-runtime` to full T1, and core 0x44587c (deferred on QPC since the
  morning) passed 200/200 with full coverage. This
  matters most for R2, where ~310 queued functions call the heap APIs.
- **Shape gate:** a known function start that the original uses as a plain operand
  (`mov esi, 0x549dcc`) no longer counts as an extra callee in the reconstruction
  (workers had been hiding such immediates behind arithmetic).
- **Layouts:** `"type": "self"` buffer fields (a node whose link points to itself) make
  pointer chains of any depth valid; composition bug introduced and fixed within the
  hour (all 70 accepts in that window re-verified: pass; a 25-function random
  sample also passes under the new oracle).
- Layout authors L22 (core: 14 accepted), L26 (R3: 9/9 accepted + 10 weak passes
  strengthened, no mismatches). 1,005 layouts, 45 lead-verified coverage exemptions; `specs/flag_exempt.json`
  for one stack-address-dependent dispatcher.
- Core still blocked: frame drivers/orchestrators (0x418138, 0x4362cd, 0x4437d6,
  0x40f943, 0x40dfb8, 0x4346f5), 0x40563c (~40 s per case), DirectPlay paths
  (0x445a59, 0x445b9a, 0x445d60), 0x43ff93 (stream layout + 5 dead-block proofs pending).

## Update (night)

| | functions | code bytes |
|---|---|---|
| core (R1) accepted | 1,849 (incl. 4 runtime) | 262,229 of 282,758 (92.7%) |
| core deferred / todo | 33 / 9 | |
| legacy (R3) accepted | 1,136 (incl. 23 runtime) | 57,249 of ~289,000 (19.8%) |
| whole game accepted | 3,044 | 319,755 of 1,687,127 (19.0%) |

- **Core:** L30 converted 6 deferrals (0x43ff93 racing-line pass, 0x408d6d race-start grid,
  0x443148, 0x4430d1, 0x420ed6, 0x4440be); 0x41cde4 passed once the never-written divisor
  0x60200c was added to `specs/constants.json`. Remaining core deferrals are the frame
  drivers/orchestrators (T2 or snapshot oracle), 0x40563c (~40 s per case), DirectPlay, and
  candidates blocked by divide-by-zero inputs (L34 on 0x4198c7/0x41e192).
- **R3 analysis:** `tools/fix_switches.py --closure` follows add-table switches by
  reachability and turned 36 case bodies shared by several functions (the menu state machines)
  into functions; ~40 more junk rows excluded (mid-instruction starts, code-address tables,
  text, a D3DX texture-loader cluster at 0x55f41d..0x560000 now classified as library).
- **Verifier:** dead-callee shape exemptions (`specs/shape_exempt.json` "callees", for calls
  only on lead-proven dead paths); global layout `alias_always`; hex bounds in `rel[...]`.
- **Layout authors** L26-L35 (R3) converted almost every layout deferral and re-ran ~60 weak
  passes with stronger domains; L27/L28 found 6 real mismatches in worker candidates (all
  fixed by targeted retries). 72 coverage exemptions, 18 constant ranges, ~1,180 layouts.

## Update (late night): correctness fixes

- **Audit of every `accepted-runtime` function** (emulator skipped, shape-only; report
  `build/audit_runtime.md`): one was wrong (0x545fa5 returned a value the original does not
  produce) and two overclaimed outputs that are Win32 leftovers (0x53657e, 0x54e33d). New runtime
  call `gp4::call_regs_stdcall(addr, regs, args)` (gp4_rt.cpp, native-tested; no static state)
  calls a stdcall routine with live registers and returns its registers/flags; all three now use
  it, and 0x545fa5 passes the full emulator comparison.
- **Oracle bug fixed:** Unicorn 2.1.x lost CF/AF across `rep movs/stos/lods` when hooks are
  installed (false NO MATCH after `add ...; rep movsb`). The emulator now saves EFLAGS at those
  sites (pre-scanned, 0.03 s) and restores the arithmetic flags; regression test added.
- **Shape gate:** excluded data rows (strings, tables) no longer count as callees. A shape-only
  pass over all 3,342 accepted functions after the analysis repairs: 0 failures.
- **False starts:** tables of 16-bit value pairs read as pointers (0x00530052 = words 0x52,0x53)
  had created starts; 4 such false accepts were reverted (sources moved to `build/removed/`) and
  the real routines they sat in registered. ~30 more of the same form remain in R2/R5 (todo).
- New layout options: `prefix` (table filled up to n entries), `alias_always`; shape exemptions
  for callees on lead-proven dead paths. 80+ lead-verified coverage exemptions in total.

## Session end (2026-10-20 evening, stopped by the user)

- Ledger: 3,953 accepted (T1), 149 accepted-runtime (T2 pending), 82 deferred, 5,954 todo; 23.85% of game
  code bytes. R1 core 93.8% of its bytes; R3 legacy 47.8% (969 todo, 51 deferred). All claims released.
- Oracle/tooling added this session: Win32 models (fake file system with caller-opened handles 0x3000..,
  Global*, GetLocalTime/GetSystemTime, MapVirtualKeyA, GetKeyNameTextA, MessageBox, PostMessageA, cursor,
  GDI text/colour, GetLogicalDrives/GetDriveTypeA; PeekMessageA deliberately NOT modelled), CRT
  _cfltcvt_init in the oracle init calls, add-table switches in the CFG, data rows no longer split CFGs,
  `emu=skip` now still compares cases that avoid the skipped API, layout `pool` for big pointer tables,
  specs/heavy.json few-case runs, tools/flag_live.py (flag liveness), tools/readme_status.py (README status
  from the ledger), lead_retry prints ACCEPTED-RUNTIME separately.
- Next: build/defer_next.txt and build/weak5.txt (local, git-ignored) hold the open layout requests and weak
  accepts; the text-script table layout script_all_00835846 unblocks most text users; R2 prep: fix the
  0x4fe2e0 row extent and the gap/u16-pair false starts before the renderer goal.
