#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00520023
// Carry-flag predicate: CF = 1 when the byte at 0x7ad8b2 is 0xff and at least two of the 22 bytes
// at 0x7c2a48 have their top bit set; otherwise CF = 0. The pushal/popal wrapper preserves every
// GPR, and the other arithmetic flags are those of the last flag writer (`cmp byte [0x7ad8b2], 0xff`
// when it differs, otherwise `cmp ebp, 2`); clc/stc then replace CF.
void __cdecl FUN_00520023(gp4::Regs* r) {
    uint32_t arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007ad8b2), 0xffu);
    uint32_t carry = 0u;
    if (GP4_GLOBAL(uint8_t, 0x007ad8b2) == 0xffu) {
        const uint8_t* bytes = GP4_ARRAY(uint8_t, 0x007c2a48);
        uint32_t negatives = 0u;
        for (int i = 0; i < 0x16; ++i) {
            if (bytes[i] & 0x80u) {
                negatives += 1u;
            }
        }
        arith = gp4::flags::sub32(negatives, 2u);
        carry = (negatives >= 2u) ? 1u : 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~1u) | carry);
}
GP4_IMPL(0x00520023, FUN_00520023, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
