#include <gp4/gp4.h>
#include <gp4/flags.h>

// Carry-flag predicate: CF is set only when the byte at 0x0093250d is nonzero and the signed word at
// 0x00875e96 is below 0x2000 (`stc`); every other path ends with `clc`. The other flags are those of the
// byte test, or of the word compare when the byte is nonzero.
// FUNCTION: GP4 0x005517ad
void __cdecl FUN_005517ad(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x0093250d);
    uint32_t flags = gp4::flags::logic8(enabled);
    uint32_t carry = 0u;
    if (enabled != 0u) {
        const int16_t level = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00875e96));
        flags = gp4::flags::sub16(static_cast<uint16_t>(level), 0x2000u);
        if (level < 0x2000) {
            carry = gp4::flags::CF;
        }
    }
    flags = (flags & ~gp4::flags::CF) | carry;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005517ad, FUN_005517ad, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
