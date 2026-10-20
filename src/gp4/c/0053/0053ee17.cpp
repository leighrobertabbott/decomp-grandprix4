#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x007a9794 is 0xff: clears the byte 0x00822272 and runs 0x00549e2c (a pushal/popal
// routine that posts the wake-up message and sleeps 4 ms, reaching the message pump). Every register is
// preserved; the flags are those of `cmp byte [0x7a9794], 0xff`, or of the callee when it runs.
// FUNCTION: GP4 0x0053ee17
void __cdecl FUN_0053ee17(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007a9794);
    uint32_t arith = gp4::flags::sub8(mode, 0xffu);
    if (mode == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x00822272) = 0u;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00549e2c, c);
        arith = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053ee17, FUN_0053ee17, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
