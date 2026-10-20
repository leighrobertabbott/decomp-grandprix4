#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00436ecb
// Adds [0x00629df8] to the dwords at +0xa4 of the esi and ebx records and [0x00629dfc] to the
// dwords at +0xa8. Leaves eax = [0x00629dfc] and the flags of the final `add [ebx+0xa8], eax`.
void __cdecl FUN_00436ecb(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    uint32_t delta = GP4_GLOBAL(uint32_t, 0x00629df8);
    GP4_FIELD(uint32_t, first, 0xa4) += delta;
    GP4_FIELD(uint32_t, second, 0xa4) += delta;

    delta = GP4_GLOBAL(uint32_t, 0x00629dfc);
    GP4_FIELD(uint32_t, first, 0xa8) += delta;
    const uint32_t before = GP4_FIELD(uint32_t, second, 0xa8);
    GP4_FIELD(uint32_t, second, 0xa8) = before + delta;

    r->eax = delta;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(before, delta);
}
GP4_IMPL(0x00436ecb, FUN_00436ecb, "regs(ebx:ptr[0xb0]:bytes, esi:ptr[0xb0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
