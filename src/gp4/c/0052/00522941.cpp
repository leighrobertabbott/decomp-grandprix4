#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the driver flag bytes at 0x007c2a48..+0x15 with bit 7 set: CF = 1 (stc) when two or more are
// set, 0 (clc) otherwise. pushal/popal wrapper: every GPR is restored; the other flags are those of
// the final "cmp ebp, 1" (popal does not restore flags), only CF is replaced.
// FUNCTION: GP4 0x00522941
void __cdecl FUN_00522941(gp4::Regs* r) {
    const uint8_t* bytes = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint32_t negatives = 0u;
    for (uint32_t i = 0u; i != 0x16u; ++i) {
        if ((bytes[i] & 0x80u) != 0u) {
            negatives += 1u;
        }
    }
    const uint32_t carry = (static_cast<int32_t>(negatives) > 1) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::sub32(negatives, 1u) & ~1u) | carry);
}
GP4_IMPL(0x00522941, FUN_00522941, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=neg22_00520023")
