#include <gp4/gp4.h>
#include <gp4/x87.h>

// Scales the top byte (alpha) of a 32-bit ARGB colour by a float factor: the byte times `scale`
// stays at extended precision on the x87 stack and goes straight into the CRT's float-to-int
// routine 0x005daabc; the truncated result replaces the top byte and the low 24 bits are kept.
// FUNCTION: GP4 0x005ce2b0
int __stdcall FUN_005ce2b0(uint32_t color, float scale) {
    using namespace gp4::x87;
    const Ext v = ext_i64((int64_t)(color >> 24)) * ext_f32(scale);
    gp4::Regs c = {};
    c.st_in = 1;
    set_in_st(c, 0, v);
    gp4::call_regs(0x005daabc, c);
    return (int)((c.eax << 24) + (color & 0xffffff));
}
GP4_IMPL(0x005ce2b0, FUN_005ce2b0, "stdcall(u32, f32[-4..4]) -> i32")
