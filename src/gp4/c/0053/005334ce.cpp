#include <gp4/gp4.h>

// Runs the four front-end routines in order (0x00548e0b, 0x00548e2f, the menu loop 0x0054684f and the
// best-time update 0x0054f8c2). Each is a register-convention routine, so one register block is
// threaded through all four calls and handed back as the wrapper's outputs. The menu loop polls
// the message pump (via 0x0053c8af / 0x0053d350) and so reaches PeekMessageA.
// FUNCTION: GP4 0x005334ce
void __cdecl FUN_005334ce(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00548e0b, c);
    gp4::call_regs(0x00548e2f, c);
    gp4::call_regs(0x0054684f, c);
    gp4::call_regs(0x0054f8c2, c);
    *r = c;
}
GP4_IMPL(0x005334ce, FUN_005334ce, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
