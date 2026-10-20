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
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Blends eax toward a bound derived from the interpolated 16-bit pairs of the block at edi.
// For edx < 0 the bound is -(max(-min(eax, 0), interp(+0x60/+0x1c0) - 0x100) + 0x100); for
// edx >= 0 it is max(max(eax, 0), interp(+0x62/+0x1c2) - 0x100) + 0x100. The interpolations
// come from 0x403b19 / 0x403b3f. Result = bound + (((eax - bound) * ebp) >> 14). ecx and edx
// are saved and restored; the flags are those of the final "add eax, ecx".
// FUNCTION: GP4 0x00403cbf
void __cdecl FUN_00403cbf(gp4::Regs* r) {
    const int32_t input = static_cast<int32_t>(r->eax);
    const int32_t selector = static_cast<int32_t>(r->edx);
    const int32_t scale = static_cast<int32_t>(r->ebp);

    gp4::Regs callee = *r;
    uint32_t bound;
    if (selector < 0) {
        int32_t limit = input;
        if (limit > 0) limit = 0;
        limit = static_cast<int32_t>(0u - static_cast<uint32_t>(limit));
        gp4::call_regs(0x00403b19, callee);
        const int32_t interpolated = static_cast<int32_t>(callee.edx);
        if (limit < interpolated) limit = interpolated;
        bound = 0u - (static_cast<uint32_t>(limit) + 0x100u);
    } else {
        int32_t limit = input;
        if (limit < 0) limit = 0;
        gp4::call_regs(0x00403b3f, callee);
        const int32_t interpolated = static_cast<int32_t>(callee.edx);
        if (limit < interpolated) limit = interpolated;
        bound = static_cast<uint32_t>(limit) + 0x100u;
    }

    const int32_t delta = static_cast<int32_t>(r->eax - bound);
    const int64_t product = static_cast<int64_t>(delta) * static_cast<int64_t>(scale);
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t result = shifted + bound;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(shifted, bound, result);
}
GP4_IMPL(0x00403cbf, FUN_00403cbf, "regs(eax:i32, edx:i32, ebp:i32, edi:ptr[0x1d0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
