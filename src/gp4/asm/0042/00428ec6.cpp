#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00428ec6
void __cdecl FUN_00428ec6(gp4::Regs* r) {
    uint8_t* block = (uint8_t*)r->esi;
    int32_t value = (int32_t)GP4_FIELD(int16_t, block, 0x90);
    uint32_t limitField;
    if (value < 0) {
        value = -value;
        limitField = GP4_FIELD(uint16_t, block, 0x11c);
    } else {
        limitField = GP4_FIELD(uint16_t, block, 0x11e);
    }
    const int32_t limit = (int32_t)(limitField + 0x300u);

    // cmp value, limit sets PF/AF/ZF/SF/OF; the final clc/stc only rewrites CF
    const uint32_t a = (uint32_t)value;
    const uint32_t b = (uint32_t)limit;
    const uint32_t diff = a - b;
    const uint32_t flags = parityFlag(diff)
        | (((a ^ b ^ diff) & 0x10u) != 0u ? 0x10u : 0u)
        | (diff == 0u ? 0x40u : 0u)
        | ((diff & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ diff)) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (value >= limit ? 1u : 0u);

    r->eax = (uint32_t)value;
    r->edx = (uint32_t)limit;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00428ec6, FUN_00428ec6, "regs(esi:ptr[0x128]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
