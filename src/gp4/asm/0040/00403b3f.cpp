#include <gp4/gp4.h>

struct UnknownBlock_00403b3f {
    uint8_t Unknown00[0x62];
    uint16_t Unknown62;
    uint8_t Unknown64[0x15e];
    uint16_t Unknown1C2;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t SubFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (lhs < rhs ? 1u : 0u) | parityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Interpolates the 16-bit pair (+0x62, +0x1c2) of the block at edi by the 14-bit fraction
// DAT_00601b34 and returns edx = that value - 0x100. eax and ecx are preserved. The flags are
// those of the final "sub edx, 0x100".
// FUNCTION: GP4 0x00403b3f
void __cdecl FUN_00403b3f(gp4::Regs* r) {
    const UnknownBlock_00403b3f* block = (const UnknownBlock_00403b3f*)r->edi;

    const uint32_t low = block->Unknown62;
    const uint32_t high = block->Unknown1C2;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(high - low))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601b34));
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t value = shifted + low;
    const uint32_t result = value - 0x100u;

    r->edx = result;
    r->eflags = (r->eflags & ~0x8d5u) | SubFlags(value, 0x100u, result);
}
GP4_IMPL(0x00403b3f, FUN_00403b3f, "regs(edi:ptr[0x1ca]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
