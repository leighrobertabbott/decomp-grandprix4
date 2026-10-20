#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
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

// FUNCTION: GP4 0x004295e0
void __cdecl FUN_004295e0(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->edi;
    const int32_t scale = (int32_t)r->eax;
    const int32_t factor = (int32_t)GP4_FIELD(int16_t, body, 0x14);

    // imul edx (edx:eax = eax * edx), then shrd eax, edx, 14; edx is saved/restored
    const int64_t product = (int64_t)scale * (int64_t)factor;
    const uint32_t low = (uint32_t)product;
    const uint32_t high = (uint32_t)((uint64_t)product >> 32);
    const uint32_t shifted = (low >> 14) | (high << 18);

    uint32_t value = 0x400u - shifted;                // neg eax; add eax, 0x400
    const uint32_t flags = subtractionFlags32(value, 0x10u);   // cmp eax, 0x10
    if ((int32_t)value < 0x10) {
        value = 0x10u;
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004295e0, FUN_004295e0, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
