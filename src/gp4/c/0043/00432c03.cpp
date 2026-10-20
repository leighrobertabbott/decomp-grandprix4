#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

static inline uint32_t subtractionFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00432c03
void __cdecl FUN_00432c03(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;
    uint32_t eax = r->eax;
    uint32_t flags;

    if (GP4_GLOBAL(uint32_t, 0x0062d2b0) == 0u) {
        flags = 0x44u;                                // test of zero: ZF and PF
    } else {
        const uint32_t kind = GP4_FIELD(uint8_t, body, 0x7e);
        if (kind == 2u || kind == 3u) {
            flags = additionFlags32(eax, 0x160u);
            eax += 0x160u;
        } else {
            flags = subtractionFlags8(kind, 3u);      // cmp byte [esi+0x7e], 3
        }
    }

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432c03, FUN_00432c03, "regs(eax:u32, esi:ptr[0x88]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
