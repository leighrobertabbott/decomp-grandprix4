#include <gp4/gp4.h>

// `mov esi, 0x008319b9 ; call 0x0053c8c8 ; ret`: tail wrapper. esi = the screen record at 0x008319b9 goes
// to the front-end wait 0x0053c8c8, which polls 0x0053d350; that poll reaches the message pump
// 0x00509b50 (via 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the callee
// leaves them.
// FUNCTION: GP4 0x005449f2
void __cdecl FUN_005449f2(gp4::Regs* r) {
    r->esi = 0x008319b9u;
    gp4::call_regs(0x0053c8c8, *r);
}
GP4_IMPL(0x005449f2, FUN_005449f2, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
