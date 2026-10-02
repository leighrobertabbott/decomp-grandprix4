# What "verified" means in this project

A rebuilt GP4 function counts as **verified** (state `accepted`) only when the
tool itself says so. An AI worker saying "this matches" counts for nothing. This
page states the bar precisely, and what it does and does not prove.

## The checks, in order

1. **Lint.** The file names exactly one original GP4 address
   (`// FUNCTION: GP4 0x...` plus one `GP4_IMPL`). It uses none of the banned
   shortcuts: inline assembly, `volatile`, `<math.h>` or `<windows.h>`.
2. **Build.** It compiles and links with every other verified function, using
   x87 floating point like the 2002 original.
3. **Shape.** It calls the same GP4 functions and Windows APIs, and touches the
   same global data, as the original. This catches stubbed-out or re-routed
   bodies even when no test input reaches them.
4. **Side-by-side run.** The original function and the rebuilt one run in the
   same emulator on identical generated inputs: 200 cases by default, each
   input type chosen from the function's declared interface. For every case
   these must match exactly (or within a tolerance the spec states openly):
   - the return value,
   - every declared output register, x87 value and flag,
   - every byte of memory either run wrote.

A function that calls Windows can't be fully run in the emulator. It becomes
`accepted-runtime` and must later be confirmed inside the running game with the
hook DLL.

## What it proves

On every tested input, the rebuilt function produces exactly the same results
and memory changes as Crammond's original, given the same callees and globals.

## What it does not prove (yet)

- **Untested paths.** Inputs are random and shaped by the function's declared
  interface, so a branch that no generated input reaches is never compared. The
  shape check partly covers this. Measuring how much of each original function
  the tests reach is the next planned improvement.
- **The interface itself.** If a worker under-declares what a function reads or
  returns, the comparison only covers what was declared. Warnings report
  registers the original changes without declaring them.
- **Behaviour in the full game.** That is the in-game hook's job (milestones M1
  to M3 in `STRATEGY.md`).

## Staying verified

`python -m gp4re regress` re-runs every check on every accepted function. The
lead runs it at each checkpoint, and after any change to shared headers or the
runtime. Anything that stops passing is reopened.
