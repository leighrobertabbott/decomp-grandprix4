#include <gp4/gp4.h>

struct Unknown_004322a6 {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;
    uint8_t Unknown20[0x142];
    uint8_t Unknown162;
    uint8_t Unknown163[0x55];
    uint32_t Unknown1B8;
};

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t logicalFlags8(uint8_t value) {
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004322a6
void __cdecl FUN_004322a6(gp4::Regs* r) {
    Unknown_004322a6* body = reinterpret_cast<Unknown_004322a6*>(r->esi);
    const uint32_t value = body->Unknown1C;
    body->Unknown162 &= 0xf7u;
    uint32_t threshold = GP4_GLOBAL(uint32_t, 0x00891500);
    uint32_t flags = comparisonFlags32(value, threshold);
    if (value >= threshold) {
        threshold = GP4_GLOBAL(uint32_t, 0x00868520);
        flags = comparisonFlags32(value, threshold);
        if (value >= threshold) {
            threshold = GP4_GLOBAL(uint32_t, 0x00868524);
            flags = comparisonFlags32(value, threshold);
            if (value < threshold) {
                body->Unknown162 |= 8u;
                flags = logicalFlags8(body->Unknown162);
            }
        }
    }
    body->Unknown1B8 = value;
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004322a6, FUN_004322a6, "regs(esi:ptr[0x1c0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ranges_004322a6")
