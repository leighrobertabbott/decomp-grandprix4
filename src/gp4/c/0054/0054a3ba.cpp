#include <gp4/gp4.h>
#include <gp4/flags.h>

// Forces the link-down state (the byte [0x007aafd3] = 0, [0x007aafd8] = 0xff) and runs the screen/menu routine
// 0x0053c8c8 on the static record esi = 0x008331ac. Registers and flags come back from that routine.
// FUNCTION: GP4 0x0054a3ba
void __cdecl FUN_0054a3ba(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007aafd3) = 0u;
    GP4_GLOBAL(uint8_t, 0x007aafd8) = 0xffu;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x008331acu;
    gp4::call_regs(0x0053c8c8, c);
    *r = c;
}
GP4_IMPL(0x0054a3ba, FUN_0054a3ba, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
