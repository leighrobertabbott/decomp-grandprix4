#include <gp4/gp4.h>

// Posts the window message 0x404 to the main window with 0x0052cbb3, then clears the bytes 0x00824569
// and 0x0082ff13. Registers and flags come back as the callee leaves them (the stores touch none).
// FUNCTION: GP4 0x00549d12
void __cdecl FUN_00549d12(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052cbb3, c);
    GP4_GLOBAL(uint8_t, 0x00824569) = 0u;
    GP4_GLOBAL(uint8_t, 0x0082ff13) = 0u;
    *r = c;
}
GP4_IMPL(0x00549d12, FUN_00549d12, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
