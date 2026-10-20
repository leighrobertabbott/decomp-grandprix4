#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: asks the bit-set synchroniser 0x00510e2d about bit 0x78 (cl = 0x78). It reports ZF
// when it just set the state bit (cl = 0); only then the byte 0x00852da5 is inverted. popal restores
// every register; the flags are the synchroniser's, or those of the `xor byte [0x852da5], 0xff`.
// FUNCTION: GP4 0x0051d5f2
void __cdecl FUN_0051d5f2(gp4::Regs* r) {
    gp4::Regs c = {};
    c.ecx = 0x78u;
    gp4::call_regs(0x00510e2d, c);
    uint32_t arith = c.eflags;
    if ((c.eflags & gp4::flags::ZF) != 0u) {
        const uint8_t inverted = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00852da5) ^ 0xffu);
        GP4_GLOBAL(uint8_t, 0x00852da5) = inverted;
        arith = gp4::flags::logic8(inverted);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051d5f2, FUN_0051d5f2, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_0051d5f2")
