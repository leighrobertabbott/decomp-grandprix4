#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frame-limiter wait: unless the mode byte 0x007aafdb is 1, stores the timer fraction of 0x0054e6af
// in 0x006f2c00, then polls 0x0054e6af until it reaches the limit dword 0x007ad8b8. The routine ends
// with eax = the limit, edx = the last timer value and the flags of `cmp eax, edx`; with mode 1 only
// the flags of `cmp byte [0x7aafdb], 1` are left.
// FUNCTION: GP4 0x0051d9c8
void __cdecl FUN_0051d9c8(gp4::Regs* r) {
    uint32_t flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007aafdb), 1u);
    if (GP4_GLOBAL(uint8_t, 0x007aafdb) != 1u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x0054e6af, c);
        GP4_GLOBAL(uint32_t, 0x006f2c00) = c.eax;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x007ad8b8);
        uint32_t now;
        do {
            c.eax = limit;
            gp4::call_regs(0x0054e6af, c);
            now = c.eax;
        } while (limit > now);
        r->eax = limit;
        r->edx = now;
        flags = gp4::flags::sub32(limit, now);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051d9c8, FUN_0051d9c8, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rate_007ad8b8")
