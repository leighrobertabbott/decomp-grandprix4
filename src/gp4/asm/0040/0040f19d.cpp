#include <gp4/gp4.h>

struct Unknown_0040f19d {
    uint8_t Unknown00[0x18];
    uint16_t Unknown18;
    uint8_t Unknown1A[2];
    const uint16_t* Unknown1C;
    uint8_t Unknown20[0x7a];
    uint16_t Unknown9A;
};

static inline uint32_t comparisonFlags(uint32_t left, uint32_t right, uint32_t sign, uint32_t mask) {
    const uint32_t result = (left - right) & mask;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040f19d
void __cdecl FUN_0040f19d(gp4::Regs* r) {
    const Unknown_0040f19d* body = reinterpret_cast<const Unknown_0040f19d*>(r->esi);
    const uint16_t value = body->Unknown9A;
    uint32_t flags = comparisonFlags(value, 94u, 0x8000u, 0xffffu);
    if (static_cast<int16_t>(value) < 94) {
        flags |= 1u;
    } else {
        const uint16_t differenceBits = static_cast<uint16_t>(body->Unknown18 - *body->Unknown1C);
        const int32_t difference = static_cast<int16_t>(differenceBits);
        const uint32_t magnitude = static_cast<uint32_t>(difference < 0 ? -difference : difference);
        flags = comparisonFlags(magnitude, 0x4000u, 0x80000000u, 0xffffffffu);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f19d, FUN_0040f19d, "regs(esi:ptr[0xa2]:layout=body_link_0040f19d) -> (cf, pf, af, zf, sf, of)")
