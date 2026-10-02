#include <gp4/gp4.h>

struct Unknown_00435854 {
    uint8_t Unknown00[0x218];
    uint8_t Unknown218[4];
    uint8_t Unknown21C[0x39c - 0x21c];
    int32_t Unknown39C[4];
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "sar reg32, 16" under the pinned oracle model:
// CF = bit 15 of the source, AF = 0, OF = 0 (count != 1 keeps the sign).
static inline uint32_t sar16Flags(int32_t value) {
    const int32_t result = value >> 16;
    return (((uint32_t)value >> 15) & 1u)
        | parityFlag((uint32_t)result)
        | (result == 0 ? 0x40u : 0u)
        | (result < 0 ? 0x80u : 0u);
}

// Packs the high 16 bits (arithmetically shifted) of four dwords at +0x39c into the
// four bytes at +0x218.
// FUNCTION: GP4 0x00435854
void __cdecl FUN_00435854(gp4::Regs* r) {
    Unknown_00435854* body = reinterpret_cast<Unknown_00435854*>(r->esi);
    const int32_t v0 = body->Unknown39C[0];
    body->Unknown218[0] = (uint8_t)(v0 >> 16);
    const int32_t v1 = body->Unknown39C[1];
    body->Unknown218[1] = (uint8_t)(v1 >> 16);
    const int32_t v2 = body->Unknown39C[2];
    body->Unknown218[2] = (uint8_t)(v2 >> 16);
    const int32_t v3 = body->Unknown39C[3];
    body->Unknown218[3] = (uint8_t)(v3 >> 16);
    r->eflags = (r->eflags & ~0x8d5u) | sar16Flags(v3);
}
GP4_IMPL(0x00435854, FUN_00435854, "regs(esi:ptr[0x3b0]:bytes) -> (cf, pf, af, zf, sf, of)")
