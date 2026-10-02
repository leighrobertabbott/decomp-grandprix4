# Reconstruction checkpoint: 2026-10-02

The complete-reconstruction goal remains active. Active ledger goal: `core`.
Accepted functions increased from 2 to 33; 10,495 game functions remain todo.
32 of the 1,666 core functions are accepted. Accepted game bytes: 561 of
1,731,034 (0.032%; analysis extents may overlap). No current claims or
deferrals remain at this checkpoint, and no STOP file exists.

Three reconstruction workers ran in parallel within the available four-agent
limit. Worker IDs used: codex-a01, b01, b02, b03, b04, c01, c03. The three
deferrals (private-stack stub, SHRD flags, zero discriminator coverage) were
unblocked and accepted. Future worker IDs should start with `codex-d01`.

The verifier now enforces target-body block/branch coverage, missing
GPR/flag/stack input checks, complete register-block effects, compiled ABI
preservation and live caller-stack effects. It checks raw x87 outputs, rejects
invalid original cases and failed initialization, traps out-of-scope wrapper
execution, requires exact call/import targets and evidenced runtime skips,
and does not apply float tolerance to raw memory. Ranged integer generation
includes endpoints and -1/0/1. Global layouts vary evidenced fields without
changing neighbours; scalar dword getters/setters use `globals=fuzz:i32`.

The hook adapter originally changed incoming flags before capturing them.
LEA allocation fixed that; native tests cover all 64 arithmetic-flag
combinations. New executable synthetic tests reproduce previously false
passes and lock in the stronger gates. Validation: 45 tests passed, and every
accepted function passed regression with 200 cases. One source audit checked
0x004052ef against its disassembly and confirmed modulo arithmetic, original
load widths, call-free structure and the documented pinned-Unicorn flag model.

Original GP4.exe SHA-256 remains
`2046f3902246b2aff48dca2e4f7e8c049d410166e60a5dc20fe3750f4a926b12`.
The user's Grand-Prix-4 install was only read. Progress was exported through
`gp4re export`. The initial repository has no commits; GP4RE_AUTOCOMMIT is
unset, so no commit was made.

Next: continue size bands within core (head <=48, main/asm 49..400, then big).
Give workers the updated protocol and keep supplied field layouts under lead
control. Outstanding verifier limits are arbitrary extended x87 inputs and
complete status/control-state adapters, global non-interference, alias
topologies and MMX/XMM state; native undefined flag behavior also needs T2.
Do not mark the full goal complete or start libgp4sim extraction before the
accepted simulation closure is ready.
