#include <gp4/gp4.h>

struct Unknown_0040ed29 {
    uint8_t Unknown00[0x70];
    uint8_t Counter70;
    uint8_t Unknown71[0x7c - 0x71];
    uint8_t Unknown7C;
    uint8_t Unknown7D[0x415 - 0x7d];
    uint8_t Unknown415;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "add byte ptr [mem], 1"
static inline uint32_t incByteFlags(uint8_t old) {
    const uint32_t result = (uint8_t)(old + 1u);
    return (result < old ? 1u : 0u)
        | parityFlag(result)
        | (((old ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((~(old ^ 1u) & (old ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Advances the counter at +0x70 and tallies the record's 6-bit kind (1-based, clamped to 0x15)
// in the byte table at 0x66ec4a; when +0x415 is clear it also records the counter's old value
// (if not above the word at 0x7c2e96) in 0x66d8c8 / 0x7c2e9c.
// FUNCTION: GP4 0x0040ed29
void __cdecl FUN_0040ed29(gp4::Regs* r) {
    Unknown_0040ed29* body = reinterpret_cast<Unknown_0040ed29*>(r->esi);

    if (body->Unknown415 == 0u) {
        const uint32_t current = body->Counter70;
        const uint32_t limit = GP4_GLOBAL(uint16_t, 0x007c2e96);
        r->edx = limit;
        if (current <= limit) {
            GP4_GLOBAL(uint8_t, 0x0066d8c8) = (uint8_t)current;
            GP4_GLOBAL(uint16_t, 0x007c2e9c) = (uint16_t)current;
        }
    }

    body->Counter70 = (uint8_t)(body->Counter70 + 1u);

    int32_t index = (int32_t)(body->Unknown7C & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    uint8_t* cell = GP4_ARRAY(uint8_t, 0x0066ec4a) + index;
    const uint8_t old = *cell;
    *cell = (uint8_t)(old + 1u);

    r->eax = (uint32_t)index;
    r->eflags = (r->eflags & ~0x8d5u) | incByteFlags(old);
}
GP4_IMPL(0x0040ed29, FUN_0040ed29, "regs(esi:ptr[0x41d]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
