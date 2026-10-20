#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x0088f6e1 is set: clears it and runs 0x00518479 (which then sees the byte clear).
// A clear byte leaves the flags of `test byte, 0xff`; otherwise the flags are those the callee leaves
// (the store clears none).
// FUNCTION: GP4 0x005188a6
void __cdecl FUN_005188a6(gp4::Regs* r) {
    const uint32_t flag = GP4_GLOBAL(uint8_t, 0x0088f6e1);
    uint32_t arith = gp4::flags::logic8(flag);
    if (flag != 0u) {
        GP4_GLOBAL(uint8_t, 0x0088f6e1) = 0u;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00518479, c);
        arith = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005188a6, FUN_005188a6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
