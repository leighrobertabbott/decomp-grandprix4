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

// Blends eax toward a bound taken from the block at edi: the sign of edx picks which of the two
// 16-bit pairs (+0x62/+0x1c2 for edx >= 0, +0x60/+0x1c0 for edx < 0) is interpolated by the
// 14-bit fraction DAT_00601b34 (via 0x403b3f / 0x403b19). The bound is
// max(clamp(eax), interpolated - 0x100) + 0x100 (negated for edx < 0); the result is
// bound + (((eax - bound) * ebp) >> 14). ecx and edx are preserved; the flags are those of the
// final "add eax, ecx".
// FUNCTION: GP4 0x00403b65
void __cdecl FUN_00403b65(gp4::Regs* r) {
    const int32_t input = static_cast<int32_t>(r->eax);
    const int32_t selector = static_cast<int32_t>(r->edx);
    const int32_t scale = static_cast<int32_t>(r->ebp);

    gp4::Regs callee = *r;
    uint32_t bound;
    if (selector < 0) {
        const int32_t clamped = input > 0 ? 0 : input;
        const int32_t negated = static_cast<int32_t>(0u - static_cast<uint32_t>(clamped));
        gp4::call_regs(0x00403b19, callee);
        const int32_t blocked = static_cast<int32_t>(callee.edx);
        const int32_t high = negated < blocked ? blocked : negated;
        bound = 0u - (static_cast<uint32_t>(high) + 0x100u);
    } else {
        const int32_t clamped = input < 0 ? 0 : input;
        gp4::call_regs(0x00403b3f, callee);
        const int32_t blocked = static_cast<int32_t>(callee.edx);
        const int32_t high = clamped < blocked ? blocked : clamped;
        bound = static_cast<uint32_t>(high) + 0x100u;
    }

    const int32_t delta = static_cast<int32_t>(r->eax - bound);
    const int64_t product = static_cast<int64_t>(delta) * static_cast<int64_t>(scale);
    const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t result = shifted + bound;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(shifted, bound, result);
}
GP4_IMPL(0x00403b65, FUN_00403b65, "regs(eax:i32, edx:i32, ebp:i32, edi:ptr[0x1d0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
