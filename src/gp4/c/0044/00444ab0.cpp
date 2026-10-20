#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit add.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// A 4-dword entry: three components and their common weight.
struct Unknown_0062a5ac {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    int32_t Unknown0C;
};

// FUNCTION: GP4 0x00444ab0
// Pre-multiplies the 23 entries of the table at 0x0062a5ac by their weight (dword +0xc): the
// components are written to 0x0062a7cc as component * weight (low 32 bits), the weight is copied.
// pushal/popal restore every register; the flags are those of the final `add edi, 0x10`.
void __cdecl FUN_00444ab0(gp4::Regs* r) {
    const Unknown_0062a5ac* source = GP4_ARRAY(Unknown_0062a5ac, 0x0062a5ac);
    Unknown_0062a5ac* target = GP4_ARRAY(Unknown_0062a5ac, 0x0062a7cc);
    for (uint32_t i = 0; i != 0x17u; ++i) {
        const uint32_t weight = static_cast<uint32_t>(source->Unknown0C);
        target->Unknown00 = static_cast<int32_t>(static_cast<uint32_t>(source->Unknown00) * weight);
        target->Unknown04 = static_cast<int32_t>(static_cast<uint32_t>(source->Unknown04) * weight);
        target->Unknown08 = static_cast<int32_t>(static_cast<uint32_t>(source->Unknown08) * weight);
        target->Unknown0C = static_cast<int32_t>(weight);
        ++source;
        ++target;
    }
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(0x0062a7cc + 0x16u * 0x10u, 0x10u);
}
GP4_IMPL(0x00444ab0, FUN_00444ab0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=parts_0062a5ac")
