#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: masks eight source dwords (0x007ad7f8..0x007ad840) with the mask dwords
// 0x00965580..0x0096558c into the dwords 0x0096547c..0x00965498 (the "| 1" ones keep bit 0, bit 4 of
// 0x00965480 is taken from 0x00965490, bit 1 of 0x00965498 is cleared), then runs the two table/flag
// routines 0x005516f3 and 0x00551685, sets 0x009654a0 = -1 and stores bit 4 of (0x00965480 | 0x00965488)
// into 0x00866e14. Registers are restored by popal; the flags are those of the final `and eax, 0x10`.
// FUNCTION: GP4 0x005515bc
void __cdecl FUN_005515bc(gp4::Regs* r) {
    const uint32_t m580 = GP4_GLOBAL(uint32_t, 0x00965580);
    GP4_GLOBAL(uint32_t, 0x0096547c) = GP4_GLOBAL(uint32_t, 0x007ad83c) & m580;
    GP4_GLOBAL(uint32_t, 0x00965484) = GP4_GLOBAL(uint32_t, 0x007ad808) & GP4_GLOBAL(uint32_t, 0x00965580);
    GP4_GLOBAL(uint32_t, 0x0096548c) = GP4_GLOBAL(uint32_t, 0x007ad7f8) & GP4_GLOBAL(uint32_t, 0x00965588);
    GP4_GLOBAL(uint32_t, 0x00965494) = GP4_GLOBAL(uint32_t, 0x007ad800) & GP4_GLOBAL(uint32_t, 0x00965588);
    GP4_GLOBAL(uint32_t, 0x00965480) = (GP4_GLOBAL(uint32_t, 0x007ad840) | 1u) & GP4_GLOBAL(uint32_t, 0x00965584);
    GP4_GLOBAL(uint32_t, 0x00965488) = (GP4_GLOBAL(uint32_t, 0x007ad80c) | 1u) & GP4_GLOBAL(uint32_t, 0x00965584);
    const uint32_t v490 = (GP4_GLOBAL(uint32_t, 0x007ad7fc) | 1u) & GP4_GLOBAL(uint32_t, 0x0096558c);
    GP4_GLOBAL(uint32_t, 0x00965490) = v490;
    GP4_GLOBAL(uint32_t, 0x00965480) &= 0xffffffefu;
    GP4_GLOBAL(uint32_t, 0x00965480) |= v490 & 0x10u;
    GP4_GLOBAL(uint32_t, 0x00965498) = (GP4_GLOBAL(uint32_t, 0x007ad804) | 1u) & GP4_GLOBAL(uint32_t, 0x0096558c) & 0xfffffffdu;
    gp4::Regs c = *r;
    gp4::call_regs(0x005516f3, c);
    gp4::call_regs(0x00551685, c);
    GP4_GLOBAL(uint32_t, 0x009654a0) = 0xffffffffu;
    const uint32_t bit = (GP4_GLOBAL(uint32_t, 0x00965480) | GP4_GLOBAL(uint32_t, 0x00965488)) & 0x10u;
    GP4_GLOBAL(uint32_t, 0x00866e14) = bit;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(bit));
}
GP4_IMPL(0x005515bc, FUN_005515bc, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
