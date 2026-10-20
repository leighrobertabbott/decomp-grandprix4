#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x0081e4fd is 0xff: brackets a wait with the two message-pump markers 0x0053edec /
// 0x0053ee17 (pushal/popal around the whole block). If the byte 0x007ab172 (a tick count in tenths of
// a unit) is nonzero the wait routine 0x0052cb10 runs with eax = byte * 0x12 / 0xa, ebx = 0xa and
// edx = the remainder. popal restores every GPR; the flags are those 0x0053ee17 leaves (it rewrites
// all six arithmetic flags), or those of `cmp byte [0x81e4fd], 0xff` when the guard fails.
// FUNCTION: GP4 0x0053d28d
void __cdecl FUN_0053d28d(gp4::Regs* r) {
    const uint32_t guard = GP4_GLOBAL(uint8_t, 0x0081e4fd);
    const uint32_t cmp = gp4::flags::sub8(guard, 0xffu);
    if (guard != 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, cmp);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, cmp);
    gp4::call_regs(0x0053edec, c);

    const uint32_t ticks = GP4_GLOBAL(uint8_t, 0x007ab172);
    c.eax = ticks;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(ticks));
    if (ticks != 0u) {
        const uint32_t product = ticks * 0x12u;
        c.eax = product / 0xau;
        c.ebx = 0xau;
        c.edx = product % 0xau;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(product));
        gp4::call_regs(0x0052cb10, c);
    }
    gp4::call_regs(0x0053ee17, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053d28d, FUN_0053d28d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
