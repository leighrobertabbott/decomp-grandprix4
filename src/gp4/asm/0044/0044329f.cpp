#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit TEST byte, 0xff (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint8_t value) {
    return byteParity(value)
        | (value == 0u ? 0x040u : 0u)
        | ((value & 0x80u) != 0u ? 0x080u : 0u);
}

// Tests whether slot (al & 0x3f) - 1 (clamped to 0) is unflagged and has both keys at 0x20000000; the
// result is the carry flag, the other flags are those of the last compare/test.
// FUNCTION: GP4 0x0044329f
void __cdecl FUN_0044329f(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint8_t code = static_cast<uint8_t>(value);
    uint32_t flags;
    uint32_t carry = 0;

    if ((code & 0x80u) != 0u) {
        flags = testFlags8(code);
    } else {
        int32_t index = static_cast<int32_t>(value & 0x3fu) - 1;
        if (index < 0) {
            index = 0;
        }
        const uint32_t first = GP4_ARRAY(uint32_t, 0x00675970)[index];
        if (first != 0x20000000u) {
            flags = cmpFlags32(first, 0x20000000u);
        } else {
            const uint32_t second = GP4_ARRAY(uint32_t, 0x007c3184)[index];
            if (second != 0x20000000u) {
                flags = cmpFlags32(second, 0x20000000u);
            } else {
                const uint8_t flag = GP4_ARRAY(uint8_t, 0x0066d90a)[index];
                flags = testFlags8(flag);
                if ((flag & 0x80u) == 0u) {
                    carry = 1u;
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x0044329f, FUN_0044329f, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=flags_0066d909+keys_0044329f")
