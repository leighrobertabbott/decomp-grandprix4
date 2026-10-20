#include <gp4/gp4.h>

// Runs the empty routine 0x00527f80 with eax = 0x31a, then 0x0054a3ba and 0x0054a25d (the latter
// polls the message pump, so PeekMessageA). Registers and flags come back as 0x0054a25d leaves them.
// FUNCTION: GP4 0x00532567
void __cdecl FUN_00532567(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x31au;
    gp4::call_regs(0x00527f80, c);
    gp4::call_regs(0x0054a3ba, c);
    gp4::call_regs(0x0054a25d, c);
    *r = c;
}
GP4_IMPL(0x00532567, FUN_00532567, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
