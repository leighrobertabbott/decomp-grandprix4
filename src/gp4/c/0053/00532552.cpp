#include <gp4/gp4.h>

// A front-end wait: runs the empty routine 0x00527f80 with eax = 0x31a, then the wait 0x0053c8af with
// esi = 0x0084c5a8 (the item record). The poll reaches the message pump (via 0x0053d350) and so
// PeekMessageA. Registers and flags come back as 0x0053c8af leaves them.
// FUNCTION: GP4 0x00532552
void __cdecl FUN_00532552(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x31au;
    gp4::call_regs(0x00527f80, c);
    c.esi = 0x0084c5a8u;
    gp4::call_regs(0x0053c8af, c);
    *r = c;
}
GP4_IMPL(0x00532552, FUN_00532552, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
