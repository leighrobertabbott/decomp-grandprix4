#include <gp4/gp4.h>
#include <gp4/flags.h>

// Calls the empty routine 0x00527f80 with eax = 0x2e5, then the front-end poll 0x0053c8af with esi = 0x008427e0
// (the menu record). The poll waits through the message pump 0x00509b50 (via 0x0053d350) and so reaches
// PeekMessageA. Registers and flags come back as the poll leaves them.
// FUNCTION: GP4 0x00533201
void __cdecl FUN_00533201(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x2e5u;
    gp4::call_regs(0x00527f80, c);
    c.esi = 0x008427e0u;
    gp4::call_regs(0x0053c8af, c);
    *r = c;
}
GP4_IMPL(0x00533201, FUN_00533201, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
