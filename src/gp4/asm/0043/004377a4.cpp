#include <gp4/gp4.h>

struct Unknown_004377a4 {
    uint8_t Unknown00[0xe];
    uint16_t Unknown0E;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (dword).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// |angle| (16-bit) looked up in the Q14 table at 0x007c5c5c (word at byte offset
// (|angle| >> 2) & 0xfffe) times a signed 16-bit factor: the 32-bit product of "imul word".
static inline uint32_t tableProduct(uint16_t angle, int16_t factor) {
    if ((int16_t)angle < 0) {
        angle = (uint16_t)(0u - angle);
    }
    const uint32_t offset = (uint32_t)(angle >> 2) & 0xfffeu;
    const int16_t entry = GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1];
    return (uint32_t)((int32_t)entry * (int32_t)factor);
}

// FUNCTION: GP4 0x004377a4
// Projects the two factors [0x934df4] and [0x934df0] on the angle word at +0xe of the record in esi
// (sin and cos from the Q14 table, summed and scaled by 4), zeroing results whose magnitude is
// below 0x100. The word goes to [0x629df4] when esi equals [0x629e00], else to [0x629df6].
// pushal/popal restores every register; the flags left are those of "cmp esi, [0x629e00]".
void __cdecl FUN_004377a4(gp4::Regs* r) {
    const Unknown_004377a4* body = reinterpret_cast<const Unknown_004377a4*>(r->esi);
    const uint16_t angle = body->Unknown0E;

    const uint32_t first = tableProduct(angle, GP4_GLOBAL(int16_t, 0x00934df4));
    const uint32_t second = tableProduct((uint16_t)(0x4000u - angle), GP4_GLOBAL(int16_t, 0x00934df0));
    uint16_t value = (uint16_t)(((second + first) << 2) >> 16);

    uint16_t magnitude = value;
    if ((int16_t)magnitude < 0) {
        magnitude = (uint16_t)(0u - magnitude);
    }
    if ((int16_t)magnitude < 0x100) {
        value = 0;
    }

    const uint32_t flags = cmpFlags32(r->esi, GP4_GLOBAL(uint32_t, 0x00629e00));
    if (r->esi == GP4_GLOBAL(uint32_t, 0x00629e00)) {
        GP4_GLOBAL(uint16_t, 0x00629df4) = value;
    } else {
        GP4_GLOBAL(uint16_t, 0x00629df6) = value;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004377a4, FUN_004377a4, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_004377a4")
