#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "shl reg32, count" for count in 2..31 under the pinned oracle model:
// CF = last bit shifted out, AF = 0, OF = bit31((value << (count-1)) ^ result).
static inline uint32_t shlFlags(uint32_t value, uint32_t count) {
    const uint32_t before = value << (count - 1u);
    const uint32_t result = value << count;
    return ((before >> 31) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((before ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
}

// edx:eax / divisor as the x86 "div" does when edx < divisor (otherwise the CPU faults).
static inline void div64by32(uint32_t hi, uint32_t lo, uint32_t divisor, uint32_t* quotient, uint32_t* remainder) {
    uint32_t rem = hi;
    uint32_t quot = 0u;
    for (int bit = 31; bit >= 0; bit--) {
        const uint32_t carry = rem >> 31;
        rem = (rem << 1) | ((lo >> bit) & 1u);
        quot <<= 1;
        if (carry != 0u || rem >= divisor) {
            rem -= divisor;
            quot |= 1u;
        }
    }
    *quotient = quot;
    *remainder = rem;
}

// Looks up a per-index byte (index = |signed al|, 1-based) in the table at edx and turns its
// low 7 bits, clamped to 0x10..0x50, into a divisor of DAT_00679b48 << 6.
// FUNCTION: GP4 0x0041242f
void __cdecl FUN_0041242f(gp4::Regs* r) {
    const uint8_t* table = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edx));
    int32_t index = (int8_t)(r->eax & 0xffu);
    if (index < 0) {
        index = -index;
    }

    uint32_t quotient = 0u;
    uint32_t flags = 0x44u;   // "or eax, eax" / "and eax, 0x7f" giving zero
    if (index != 0) {
        uint32_t entry = table[index - 1] & 0x7fu;
        if (entry != 0u) {
            if ((int32_t)entry > 0x50) {
                entry = 0x50u;
            }
            if ((int32_t)entry < 0x10) {
                entry = 0x10u;
            }
            const uint32_t value = GP4_GLOBAL(uint32_t, 0x00679b48);
            uint32_t remainder;
            div64by32(value >> 26, value << 6, entry, &quotient, &remainder);
            r->edx = remainder;
            // div leaves the flags of "shl eax, 6"
            flags = shlFlags(value, 6u);
        }
    }
    r->eax = quotient;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041242f, FUN_0041242f, "regs(eax:u32, edx:ptr[0x88]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=keep")
