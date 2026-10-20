#include <gp4/gp4.h>

// `call 0x0052cbc9 ; ret`: tail wrapper. The callee dispatches on [0x7aafd3] / [0x7ab020] to the menu
// event loops 0x0052cbef / 0x0052ccae, which never return until the screen is left; both reach the
// message pump 0x00509b50 (via 0x0054aff0 / 0x0054a2c3) and so PeekMessageA. Registers and flags come
// back as the callee leaves them.
// FUNCTION: GP4 0x00532354
void __cdecl FUN_00532354(gp4::Regs* r) {
    gp4::call_regs(0x0052cbc9, *r);
}
GP4_IMPL(0x00532354, FUN_00532354, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
