#include <gp4/gp4.h>

struct Unknown_00420e1d {
    uint8_t Unknown00[0x2ec];
    int32_t Unknown2EC;
    int32_t Unknown2F0;
    int32_t Unknown2F4;
    int32_t Unknown2F8;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "shrd reg32, reg32, 14" under the pinned oracle model:
// CF = last bit shifted out, AF = 0, OF = bit31(concat >> 13 xor concat >> 14).
static inline uint32_t shrd14Flags(uint32_t low, uint32_t high) {
    const uint64_t concat = ((uint64_t)high << 32) | low;
    const uint32_t previous = (uint32_t)(concat >> 13);
    const uint32_t result = (uint32_t)(concat >> 14);
    return ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0 ? 0x40u : 0u)
        | ((result & 0x80000000u) ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) ? 0x800u : 0u);
}

// Scales the sum of four counters at +0x2ec..+0x2f8 (capped by DAT_00608154) by
// DAT_006228ac / (eax << 10) and DAT_00608150 in Q14 fixed point.
// FUNCTION: GP4 0x00420e1d
void __cdecl FUN_00420e1d(gp4::Regs* r) {
    const Unknown_00420e1d* body = reinterpret_cast<const Unknown_00420e1d*>(r->esi);
    uint32_t scale = r->eax;
    if (scale == 0) {
        scale = 1;
    }
    const int32_t divisor = (int32_t)(scale << 10);

    int32_t total = (int32_t)((uint32_t)body->Unknown2EC + (uint32_t)body->Unknown2F0
        + (uint32_t)body->Unknown2F4 + (uint32_t)body->Unknown2F8);
    if (total > GP4_GLOBAL(int32_t, 0x00608154)) {
        total = GP4_GLOBAL(int32_t, 0x00608154);
    }

    const int64_t dividend = (int64_t)total * (int64_t)GP4_GLOBAL(int32_t, 0x006228ac);
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, divisor, &remainder);

    const int64_t product = (int64_t)quotient * (int64_t)GP4_GLOBAL(int32_t, 0x00608150);
    const uint32_t low = (uint32_t)(uint64_t)product;
    const uint32_t high = (uint32_t)((uint64_t)product >> 32);

    r->eax = (low >> 14) | (high << 18);
    r->eflags = (r->eflags & ~0x8d5u) | shrd14Flags(low, high);
}
GP4_IMPL(0x00420e1d, FUN_00420e1d, "regs(eax:u32[0..0x3ff], esi:ptr[0x300]:layout=rec_00420e1d) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00420e1d")
