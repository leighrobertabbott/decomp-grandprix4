#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" without CF
static inline uint32_t cmpFlagsNoCarry(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00406d90
// Tests whether edi lies in the 0x420-byte window starting at DAT_00866ed0 or the one ending at
// DAT_00866ed4: CF set when it does, clear otherwise. The other flags are those of the last
// "cmp edi, ..." executed; eax is preserved.
void __cdecl FUN_00406d90(gp4::Regs* r) {
    const uint32_t position = r->edi;
    uint32_t bound = GP4_GLOBAL(uint32_t, 0x00866ed0);
    uint32_t flags;
    uint32_t carry;

    if (position < bound) {
        flags = cmpFlagsNoCarry(position, bound);
        carry = 0u;
    } else {
        bound += 0x420u;
        if (position < bound) {
            flags = cmpFlagsNoCarry(position, bound);
            carry = 1u;
        } else {
            bound = GP4_GLOBAL(uint32_t, 0x00866ed4);
            if (position >= bound) {
                flags = cmpFlagsNoCarry(position, bound);
                carry = 0u;
            } else {
                bound -= 0x420u;
                flags = cmpFlagsNoCarry(position, bound);
                carry = (position >= bound) ? 1u : 0u;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags | carry;
}
GP4_IMPL(0x00406d90, FUN_00406d90, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
