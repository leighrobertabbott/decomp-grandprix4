#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sorts the driver list (0x00548b68), stores the driver count (0x00548e9a), calls the empty routine 0x00527f80
// with eax = 0x318, then runs the front-end poll 0x0053c8af with esi = 0x0084d428 (the menu record). The poll
// waits through the message pump 0x00509b50 (via 0x0053d350) and so reaches PeekMessageA. Registers and flags
// come back as the poll leaves them.
// FUNCTION: GP4 0x005333f5
void __cdecl FUN_005333f5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00548b68, c);
    gp4::call_regs(0x00548e9a, c);
    c.eax = 0x318u;
    gp4::call_regs(0x00527f80, c);
    c.esi = 0x0084d428u;
    gp4::call_regs(0x0053c8af, c);
    *r = c;
}
GP4_IMPL(0x005333f5, FUN_005333f5, "regs(edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023+sort_00548b68+drv_005117d3+tabs_0043ced2+scan_00548e9a; emu=skip:win32 PeekMessageA")
