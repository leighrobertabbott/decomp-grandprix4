#include <gp4/gp4.h>

// Runs the drive check 0x005367d8 with al = 1 (it selects the "D:\grand prix 3" or "C:\" root by the
// signed byte compare against [0x008339ae] and may ask for the disc); when it reports failure through
// the carry flag the word [0x007a93e4] is set to 0x8000. Registers and flags come back as the callee
// leaves them.
// FUNCTION: GP4 0x005337ee
void __cdecl FUN_005337ee(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (c.eax & 0xffffff00u) | 1u;
    gp4::call_regs(0x005367d8, c);
    if ((c.eflags & 1u) != 0u) {
        GP4_GLOBAL(uint16_t, 0x007a93e4) = 0x8000u;
    }
    *r = c;
}
GP4_IMPL(0x005337ee, FUN_005337ee, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
