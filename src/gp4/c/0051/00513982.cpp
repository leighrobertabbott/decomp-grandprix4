#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs the 22-car-record routine 0x0040b232, then rebases three pointer tables to
// offsets: the non-zero dwords of 0x00679d20..0x00679d2c have 0x0089423c subtracted, those of
// 0x0066d818..0x0066d830 and 0x00679d2c..0x00679d3c have 0x0066feec subtracted. popal restores every
// register but not the flags, which stay those of the last `cmp esi, ebx` (equal operands).
// FUNCTION: GP4 0x00513982
void __cdecl FUN_00513982(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0040b232, c);

    for (uint32_t* p = GP4_ARRAY(uint32_t, 0x00679d20); p < GP4_ARRAY(uint32_t, 0x00679d2c); ++p) {
        if (*p != 0u) *p -= 0x0089423cu;
    }
    for (uint32_t* p = GP4_ARRAY(uint32_t, 0x0066d818); p < GP4_ARRAY(uint32_t, 0x0066d830); ++p) {
        if (*p != 0u) *p -= 0x0066feecu;
    }
    for (uint32_t* p = GP4_ARRAY(uint32_t, 0x00679d2c); p < GP4_ARRAY(uint32_t, 0x00679d3c); ++p) {
        if (*p != 0u) *p -= 0x0066feecu;
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x00679d3cu, 0x00679d3cu));
}
GP4_IMPL(0x00513982, FUN_00513982, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=links_0066fef0+ptrs_0066d818")
