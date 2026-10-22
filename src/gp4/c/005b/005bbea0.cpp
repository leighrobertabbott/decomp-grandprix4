#include <gp4/gp4.h>
#include <gp4/x87.h>

// Returns -log10(x) * scale as a truncated integer (the product stays at extended precision on the
// x87 stack and goes straight into the CRT's float-to-int routine 0x005daabc); 10000 when x is
// below the threshold at 0x005fa124.
// FUNCTION: GP4 0x005bbea0
int __stdcall FUN_005bbea0(float x) {
    using namespace gp4::x87;
    if (x < GP4_GLOBAL(float, 0x005fa124)) return 10000;
    const Ext v = fchs(fyl2x(ext_lg2(), ext_f32(x))) * ext_f32(GP4_GLOBAL(float, 0x005fa128));
    gp4::Regs c = {};
    c.st_in = 1;
    set_in_st(c, 0, v);
    gp4::call_regs(0x005daabc, c);
    return (int)c.eax;
}
GP4_IMPL(0x005bbea0, FUN_005bbea0, "stdcall(f32) -> i32")
