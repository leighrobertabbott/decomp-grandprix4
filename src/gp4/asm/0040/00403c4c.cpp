#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Blends eax toward a floor derived from the block at edi. edx < 0 selects the (+0x60, +0x1c0)
// pair via 0x403b19, edx >= 0 the (+0x62, +0x1c2) pair via 0x403b3f; the interpolated value is
// combined with eax, raised to at least DAT_00622358 (signed) and turned into the pivot ecx
// (eax - max(interp + eax, floor) for edx < 0, max(interp - eax, floor) + eax otherwise). A
// non-negative ebp then gives pivot + (((eax - pivot) * ebp) >> 14); a negative ebp uses
// DAT_00622354 * ebp >> 14 instead (added for edx < 0, subtracted for edx >= 0). ecx and edx are
// saved and restored; the flags are those of the final "add eax, ecx".
// FUNCTION: GP4 0x00403c4c
void __cdecl FUN_00403c4c(gp4::Regs* r) {
    const uint32_t input = r->eax;
    const int32_t selector = static_cast<int32_t>(r->edx);
    const int32_t scale = static_cast<int32_t>(r->ebp);

    gp4::Regs callee = *r;
    uint32_t pivot;
    if (selector < 0) {
        gp4::call_regs(0x00403b19, callee);
        int32_t sum = static_cast<int32_t>(callee.edx + input);
        if (sum < GP4_GLOBAL(int32_t, 0x00622358)) {
            sum = GP4_GLOBAL(int32_t, 0x00622358);
        }
        pivot = 0u - (static_cast<uint32_t>(sum) - input);
    } else {
        gp4::call_regs(0x00403b3f, callee);
        int32_t diff = static_cast<int32_t>(callee.edx - input);
        if (diff < GP4_GLOBAL(int32_t, 0x00622358)) {
            diff = GP4_GLOBAL(int32_t, 0x00622358);
        }
        pivot = static_cast<uint32_t>(diff) + input;
    }

    uint32_t term;
    if (scale >= 0) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(input - pivot)) * scale;
        term = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    } else {
        const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00622354)) * scale;
        term = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        if (selector >= 0) {
            term = 0u - term;
        }
    }

    r->eax = term + pivot;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(term, pivot);
}
GP4_IMPL(0x00403c4c, FUN_00403c4c, "regs(eax:i32, edx:i32, ebp:i32, edi:ptr[0x1d0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
