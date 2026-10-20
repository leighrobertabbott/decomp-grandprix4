#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frame-limiter wait followed by the timer restart: runs 0x0051d9c8 (which leaves eax = limit and
// edx = last timer value when the mode byte 0x007aafdb is not 1) and, unless that byte is 1,
// 0x0054e615 (stores the performance counter in the start stamp 0x00838c54; every register kept).
// The flags are those of `cmp byte [0x7aafdb], 1`, which 0x0054e615 leaves alone.
// FUNCTION: GP4 0x0051d9b4
void __cdecl FUN_0051d9b4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0051d9c8, c);
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007aafdb);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(mode, 1u));
    if (mode != 1u) {
        gp4::call_regs(0x0054e615, c);
    }
    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051d9b4, FUN_0051d9b4, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rate_007ad8b8")
