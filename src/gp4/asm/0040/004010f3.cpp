#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t resultFlags(uint32_t result) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004010f3
// Integer square root of an unsigned 32-bit value in eax (result in eax): the argument is first
// normalised by shifts of 2, then three Newton iterations on the halved value, then shifted back.
// Every other register is saved and restored.
void __cdecl FUN_004010f3(gp4::Regs* r) {
    uint32_t value = r->eax;
    uint32_t shifts = 0;
    uint32_t flags;

    // cmp edi,0x40000000 / inc ebp / shl edi,2 until the top two bits are not both clear
    while (value < 0x40000000u) {
        const uint32_t previous = value;
        ++shifts;
        value <<= 2;
        if (value == 0u) {
            // shl edi,2 gave zero: result 0. CF = last bit shifted out,
            // OF = bit31(previous<<1 ^ result) (pinned oracle model), AF = 0.
            flags = ((previous >> 30) & 1u)
                | resultFlags(value)
                | ((((previous << 1) ^ value) & 0x80000000u) != 0u ? 0x800u : 0u);
            r->eax = 0u;
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
    }

    const uint32_t half = value >> 1;
    uint32_t estimate = (half >> 16) + 0x8000u;
    flags = 0;
    for (int i = 0; i < 3; ++i) {
        const uint32_t quotient = half / estimate;
        const uint32_t left = estimate >> 1;
        const uint32_t sum = left + quotient;
        // add edi, eax
        flags = (sum < left ? 1u : 0u)
            | resultFlags(sum)
            | (((left ^ quotient ^ sum) & 0x10u) != 0u ? 0x10u : 0u)
            | ((~(left ^ quotient) & (left ^ sum) & 0x80000000u) != 0u ? 0x800u : 0u);
        estimate = sum;
    }

    uint32_t result = estimate;
    if (shifts != 0u) {
        // shr edi, cl
        result = estimate >> shifts;
        flags = ((estimate >> (shifts - 1u)) & 1u)
            | resultFlags(result)
            | ((((estimate >> (shifts - 1u)) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004010f3, FUN_004010f3, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
