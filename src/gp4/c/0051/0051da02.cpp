#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte at 0x007aafdb equals 1, runs the timer stamp 0x0054e615 (QueryPerformanceCounter into
// 0x00838c54). The flags are those of `cmp byte [0x7aafdb], 1`, or of the callee when it ran.
// FUNCTION: GP4 0x0051da02
void __cdecl FUN_0051da02(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007aafdb);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 1u));
    if (mode == 1u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x0054e615, c);
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
    }
}
GP4_IMPL(0x0051da02, FUN_0051da02, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
