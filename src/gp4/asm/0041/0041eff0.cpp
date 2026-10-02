#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041eff0
void __cdecl FUN_0041eff0(gp4::Regs* r) {
    const uint32_t recordOffset = r->ebx * 0x30u;
    uint32_t position = r->edx;
    if ((int32_t)position > 0xb0) {
        position = 0xb0u;
    }
    const uint32_t fraction = position & 0xfu;
    const uint32_t slot = position >> 4;

    const uint8_t* near = GP4_ARRAY(uint8_t, 0x00602c44) + recordOffset + slot * 4u;
    const uint8_t* far = GP4_ARRAY(uint8_t, 0x00602c48) + recordOffset + slot * 4u;
    const uint32_t first = *(const uint32_t*)near;

    uint32_t result = first;
    uint32_t flags = 0x44u;   // and ecx,0xf -> zero: ZF/PF set
    if (fraction != 0u) {
        const uint32_t second = *(const uint32_t*)far;
        const uint32_t delta = second - first;
        const int64_t product = (int64_t)(int32_t)delta * (int64_t)(int32_t)fraction;
        const uint32_t low = (uint32_t)product;
        const uint32_t high = (uint32_t)((uint64_t)product >> 32);
        const uint32_t interpolated = (low >> 4) | (high << 28);
        flags = addFlags32(interpolated, first);
        result = interpolated + first;
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041eff0, FUN_0041eff0, "regs(ebx:u32[0..21], edx:u32[0..300]) -> (eax, cf, pf, af, zf, sf, of)")
