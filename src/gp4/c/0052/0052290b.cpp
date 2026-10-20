#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first minimum of the 22 key dwords at 0x007c3184 among those below 0x20000000 (index 0
// when none is) and tests the flag byte 0x007c2a48[index]: CF = 1 (stc) when its bit 7 is set, 0 (clc)
// otherwise. pushal/popal wrapper: every GPR is restored; the other flags are those of
// "test byte [ebx+0x7c2a48],0xff" (popal does not restore flags), only CF is replaced.
// FUNCTION: GP4 0x0052290b
void __cdecl FUN_0052290b(gp4::Regs* r) {
    const uint32_t* keys = GP4_ARRAY(uint32_t, 0x007c3184);
    uint32_t best = 0x20000000u;
    uint32_t index = 0u;
    for (uint32_t i = 0u; i < 0x16u; ++i) {
        if (keys[i] < best) {
            best = keys[i];
            index = i;
        }
    }
    const uint8_t flag = GP4_ARRAY(uint8_t, 0x007c2a48)[index];
    const uint32_t carry = (flag & 0x80u) != 0u ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::logic8(flag) & ~1u) | carry);
}
GP4_IMPL(0x0052290b, FUN_0052290b, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=keys_0044329f+state_0043bcd4")
