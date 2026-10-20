#include <gp4/gp4.h>

struct UnknownBody_0042dee4 {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
    uint8_t Unknown92[0x1e];
    int16_t UnknownB0;
};

struct UnknownOther_0042dee4 {
    uint8_t Unknown00[0xea];
    int16_t UnknownEA;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result) & 0x8000u) != 0u) ? 0x800u : 0u);
}

// When ebx is the body held in DAT_00604280 and the word at esi+0xea is +0x10 (or -0x10), the
// held pointer is cleared once the body's word +0x90 shifted by +0x200 (or negated -0x200) is not
// below its word +0xb0. dx is used as scratch; the flags are those of the last compare.
// FUNCTION: GP4 0x0042dee4
void __cdecl FUN_0042dee4(gp4::Regs* r) {
    const UnknownBody_0042dee4* body = (const UnknownBody_0042dee4*)r->ebx;
    const UnknownOther_0042dee4* other = (const UnknownOther_0042dee4*)r->esi;

    uint32_t dx = r->edx & 0xffffu;
    const uint32_t held = GP4_GLOBAL(uint32_t, 0x00604280);
    uint32_t flags = subFlags32(r->ebx, held);

    if (r->ebx == held) {
        const uint16_t selector = static_cast<uint16_t>(other->UnknownEA);
        flags = subFlags16(selector, 0x10u);
        if (selector == 0x10u) {
            dx = (static_cast<uint32_t>(static_cast<uint16_t>(body->Unknown90)) + 0x200u) & 0xffffu;
            const uint16_t limit = static_cast<uint16_t>(body->UnknownB0);
            flags = subFlags16(dx, limit);
            if (!(static_cast<int16_t>(dx) < static_cast<int16_t>(limit))) {
                GP4_GLOBAL(uint32_t, 0x00604280) = 0u;
            }
        } else {
            flags = subFlags16(selector, 0xfff0u);
            if (selector == 0xfff0u) {
                dx = (static_cast<uint32_t>(static_cast<uint16_t>(body->Unknown90)) - 0x200u) & 0xffffu;
                dx = (0u - dx) & 0xffffu;
                const uint16_t limit = static_cast<uint16_t>(body->UnknownB0);
                flags = subFlags16(dx, limit);
                if (!(static_cast<int16_t>(dx) < static_cast<int16_t>(limit))) {
                    GP4_GLOBAL(uint32_t, 0x00604280) = 0u;
                }
            }
        }
    }

    r->edx = (r->edx & 0xffff0000u) | dx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042dee4, FUN_0042dee4, "regs(edx:u32, ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (edx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
