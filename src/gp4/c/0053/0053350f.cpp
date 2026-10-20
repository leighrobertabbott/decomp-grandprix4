#include <gp4/gp4.h>

// `call 0x005468bb ; ret`: tail wrapper. The callee builds the screen state (0x00548e66, 0x00548e0b,
// 0x00548cec), sets [0x0084954f] = 1 and loops on 0x0053c8af, dispatching through a three-entry
// jump table; the poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x0053350f
void __cdecl FUN_0053350f(gp4::Regs* r) {
    gp4::call_regs(0x005468bb, *r);
}
GP4_IMPL(0x0053350f, FUN_0053350f, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
