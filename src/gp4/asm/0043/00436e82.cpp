#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `sub left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00436e82
void __cdecl FUN_00436e82(gp4::Regs* r) {
    // For the dword pairs at +0xa4 and +0xa8 of the two blocks esi and ebx: the sum of both
    // halves (arithmetic >> 1) is stored in [0x629df8] / [0x629dfc] and subtracted from both
    // dwords. edi is saved/restored; eax/edx keep the second sum and half of ebx's dword, the
    // flags are those of the final `sub [ebx+0xa8], eax`.
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    int32_t a = GP4_FIELD(int32_t, first, 0xa4) >> 1;
    int32_t b = GP4_FIELD(int32_t, second, 0xa4) >> 1;
    int32_t sum = a + b;
    GP4_GLOBAL(int32_t, 0x00629df8) = sum;
    GP4_FIELD(uint32_t, first, 0xa4) -= static_cast<uint32_t>(sum);
    GP4_FIELD(uint32_t, second, 0xa4) -= static_cast<uint32_t>(sum);

    a = GP4_FIELD(int32_t, first, 0xa8) >> 1;
    b = GP4_FIELD(int32_t, second, 0xa8) >> 1;
    sum = a + b;
    GP4_GLOBAL(int32_t, 0x00629dfc) = sum;
    GP4_FIELD(uint32_t, first, 0xa8) -= static_cast<uint32_t>(sum);
    const uint32_t before = GP4_FIELD(uint32_t, second, 0xa8);
    GP4_FIELD(uint32_t, second, 0xa8) = before - static_cast<uint32_t>(sum);

    r->eax = static_cast<uint32_t>(sum);
    r->edx = static_cast<uint32_t>(b);
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(before, static_cast<uint32_t>(sum));
}
GP4_IMPL(0x00436e82, FUN_00436e82, "regs(ebx:ptr[0xb0]:bytes, esi:ptr[0xb0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
