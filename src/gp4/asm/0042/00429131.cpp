#include <gp4/gp4.h>

struct UnknownBlock_00429131 {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Offsets the input eax by the position value from 0x403800 (called with the record in edi) against
// a 16-bit word of the record, plus 0x50: when the byte at DAT_00868582 is negative the result is
// eax + 0x403800 - (word +0x60 + 0x50), otherwise eax + (word +0x62 + 0x50) - 0x403800.
// ecx and edx are saved and restored; the flags are those of the final "add eax, ecx".
// FUNCTION: GP4 0x00429131
void __cdecl FUN_00429131(gp4::Regs* r) {
    const UnknownBlock_00429131* block = (const UnknownBlock_00429131*)r->edi;
    const uint32_t input = r->eax;

    gp4::Regs callee = *r;
    gp4::call_regs(0x00403800, callee);
    const uint32_t position = callee.eax;

    uint32_t sum;
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
        const uint32_t word = static_cast<uint32_t>(block->Unknown60) + 0x50u;
        sum = (0u - word) + position;
    } else {
        const uint32_t word = static_cast<uint32_t>(block->Unknown62) + 0x50u;
        sum = word - position;
    }
    const uint32_t result = sum + input;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(sum, input, result);
}
GP4_IMPL(0x00429131, FUN_00429131, "regs(eax:i32, edi:ptr[0x70]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
