#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x0088f6e1 is clear: runs 0x005188bc (which then sees the byte clear and restores every
// register itself) and sets the byte to 0xff. A set byte leaves the flags of `test byte, 0xff`;
// otherwise the flags are those the callee leaves (the store changes none).
// FUNCTION: GP4 0x00518890
void __cdecl FUN_00518890(gp4::Regs* r) {
    const uint32_t flag = GP4_GLOBAL(uint8_t, 0x0088f6e1);
    uint32_t arith = gp4::flags::logic8(flag);
    if (flag == 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x005188bc, c);
        GP4_GLOBAL(uint8_t, 0x0088f6e1) = 0xffu;
        arith = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00518890, FUN_00518890, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
