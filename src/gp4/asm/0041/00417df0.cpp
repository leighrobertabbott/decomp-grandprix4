#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
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

// FUNCTION: GP4 0x00417df0
void __cdecl FUN_00417df0(gp4::Regs* r) {
    uint8_t* record = (uint8_t*)r->esi;
    uint32_t flags;

    const uint32_t masked = record[0x71] & 4u;
    if (masked != 0u) {
        // test byte,4 with the bit set: all of ZF/PF/SF/CF/OF clear
        flags = 0u;
    } else {
        const int32_t value = (int32_t)GP4_FIELD(int16_t, record, 0x9a);
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x006039f0);
        r->eax = (uint32_t)value;
        flags = compareFlags32((uint32_t)value, (uint32_t)threshold);
        if (value >= threshold) {
            record[0x83] |= 4u;
            record[0x231] |= 1u;
            const uint32_t index = GP4_GLOBAL(uint16_t, 0x007ad89e);
            const uint32_t entry = GP4_ARRAY(uint32_t, 0x00602ab8)[index];
            const uint32_t offset = GP4_GLOBAL(uint32_t, 0x0066d844);
            const uint32_t sum = entry + offset;
            flags = addFlags32(entry, offset);
            GP4_FIELD(uint32_t, record, 0x54) = sum;
            r->eax = sum;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00417df0, FUN_00417df0, "regs(esi:ptr[0x239]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
