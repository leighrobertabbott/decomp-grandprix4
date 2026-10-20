#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// FUNCTION: GP4 0x0041ef48
void __cdecl FUN_0041ef48(gp4::Regs* r) {
    const uint32_t input = r->ebp;
    uint32_t result;
    uint32_t flags;

    if (static_cast<int32_t>(input) < 0x30) {
        // below the table range: pass 0x10..0x2f through, anything lower gives 0
        flags = subFlags(input, 0x10u);
        result = (static_cast<int32_t>(input) < 0x10) ? 0u : input;
    } else {
        int32_t position = GP4_GLOBAL(int32_t, 0x00622834) - GP4_GLOBAL(int32_t, 0x00622838);
        if (position < GP4_GLOBAL(int32_t, 0x00602fa4)) {
            position = GP4_GLOBAL(int32_t, 0x00602fa4);
        }
        if (position > 0xb0) {
            position = 0xb0;
        }
        const uint32_t fraction = static_cast<uint32_t>(position) & 0xfu;
        const uint32_t slot = (static_cast<uint32_t>(position) >> 4) * 6u + GP4_GLOBAL(uint32_t, 0x00622830);
        const uint32_t base = GP4_ARRAY(uint8_t, 0x00622840)[slot];
        result = base;
        flags = 0x44u;   // and ecx,0xf -> zero: ZF/PF set
        if (fraction != 0u) {
            const uint32_t next = GP4_ARRAY(uint8_t, 0x00622846)[slot];
            const uint32_t delta = next - base;
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(delta)) * static_cast<int32_t>(fraction);
            const uint32_t interpolated = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 4);
            flags = addFlags(interpolated, base);
            result = interpolated + base;
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041ef48, FUN_0041ef48, "regs(ebp:i32[-64..300]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0041ef48")
