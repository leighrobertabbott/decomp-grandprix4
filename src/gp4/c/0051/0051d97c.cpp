#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frame-limiter tail: runs the frame wait 0x0051d9c8, then, when the 64-bit pair at 0x007a93c4 is
// nonzero, polls the timer fraction 0x0054e6af until it reaches 0x007a93c0 + 0x300. Unless the mode
// byte 0x007aafdb is 1 it finally re-stamps the timer start with 0x0054e615 (all registers kept).
// The routine ends with eax = 0 (pair zero) or the limit, edx = the last timer value (or the
// wait's edx) and the flags of `cmp byte [0x7aafdb], 1`.
// FUNCTION: GP4 0x0051d97c
void __cdecl FUN_0051d97c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0051d9c8, c);

    uint32_t eax = GP4_GLOBAL(uint32_t, 0x007a93c8) | GP4_GLOBAL(uint32_t, 0x007a93c4);
    uint32_t edx = c.edx;
    if (eax != 0u) {
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x007a93c0) + 0x300u;
        uint32_t now;
        do {
            gp4::call_regs(0x0054e6af, c);
            now = c.eax;
        } while (limit > now);
        eax = limit;
        edx = now;
    }

    uint32_t flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007aafdb), 1u);
    if (GP4_GLOBAL(uint8_t, 0x007aafdb) != 1u) {
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x0054e615, c);
        flags = c.eflags;
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051d97c, FUN_0051d97c, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rate_007ad8b8+wait_0051d97c+timer_00838c4c")
