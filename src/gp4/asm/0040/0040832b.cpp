#include <gp4/gp4.h>

struct UnknownBlock_0040832b {
    uint8_t Unknown00[0x136];
    uint8_t Unknown136;
    uint8_t Unknown137[0x61];
    uint8_t Unknown198;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// eax = max(0, byte[esi + 0x198] - byte[esi + 0x136]). edx is preserved. The flags are those of
// "cmp eax, 0" on the unclamped difference (CF = OF = AF = 0): the clamping mov leaves them.
// FUNCTION: GP4 0x0040832b
void __cdecl FUN_0040832b(gp4::Regs* r) {
    const UnknownBlock_0040832b* block = (const UnknownBlock_0040832b*)r->esi;

    const int32_t difference = (int32_t)block->Unknown198 - (int32_t)block->Unknown136;
    const uint32_t value = (uint32_t)difference;
    const uint32_t flags = parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);

    r->eax = difference < 0 ? 0u : value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040832b, FUN_0040832b, "regs(esi:ptr[0x1a0]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
