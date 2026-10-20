#include <gp4/gp4.h>

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
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

struct Unknown_00403665 {
    uint8_t Unknown00[0x16];
    int16_t Unknown16;
    uint8_t Unknown18[0x60 - 0x18];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

// Blends eax toward a bound taken from the block at edi (the integer-scaled sibling of 0x403b65
// with the 14-bit fraction in eax): the sign of edx picks the side. For edx >= 0 the bound is
// max(max(word +0x16, 0), word +0x62 - 0x100) + 0x100; for edx < 0 it is
// -(max(max(-word +0x16, 0), word +0x60 - 0x100) + 0x100). With both scaled by 256 the result is
// bound + (((word +0x16 << 8) - bound) * eax >> 14). edx = high half of the 64-bit product;
// ecx and ebp are saved and restored; the flags are those of the final "add eax, ecx".
// FUNCTION: GP4 0x00403665
void __cdecl FUN_00403665(gp4::Regs* r) {
    const Unknown_00403665* block = reinterpret_cast<const Unknown_00403665*>(static_cast<uintptr_t>(r->edi));
    const int32_t current = block->Unknown16;

    int32_t bound;
    if (static_cast<int32_t>(r->edx) < 0) {
        int32_t magnitude = current < 0 ? current : 0;
        magnitude = -magnitude;
        const int32_t floorValue = static_cast<int32_t>(block->Unknown60) - 0x100;
        if (magnitude < floorValue) {
            magnitude = floorValue;
        }
        bound = -(magnitude + 0x100);
    } else {
        int32_t magnitude = current < 0 ? 0 : current;
        const int32_t floorValue = static_cast<int32_t>(block->Unknown62) - 0x100;
        if (magnitude < floorValue) {
            magnitude = floorValue;
        }
        bound = magnitude + 0x100;
    }

    const uint32_t scaledBound = static_cast<uint32_t>(bound) << 8;
    const uint32_t scaledCurrent = static_cast<uint32_t>(current) << 8;
    const int32_t delta = static_cast<int32_t>(scaledCurrent - scaledBound);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(r->eax)) * static_cast<int64_t>(delta);
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t result = shifted + scaledBound;

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(shifted, scaledBound, result);
}
GP4_IMPL(0x00403665, FUN_00403665, "regs(eax:i32, edx:i32, edi:ptr[0x70]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
