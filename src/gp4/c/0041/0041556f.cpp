#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// eax = 0x00426947(eax = [0x6247b8], esi); when that result is above [0x604118] and
// [0x624850] >= [0x625890] (signed) it is replaced by 0x4000. edx holds [0x624850] in the
// second case; the flags are those of the last compare.
// FUNCTION: GP4 0x0041556f
void __cdecl FUN_0041556f(gp4::Regs* r) {
    r->eax = GP4_GLOBAL(uint32_t, 0x006247b8);
    gp4::call_regs(0x00426947, *r);

    const uint32_t result = r->eax;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00604118);
    uint32_t flags = subFlags(result, limit);
    if (static_cast<int32_t>(result) > static_cast<int32_t>(limit)) {
        const uint32_t edx = GP4_GLOBAL(uint32_t, 0x00624850);
        const uint32_t other = GP4_GLOBAL(uint32_t, 0x00625890);
        r->edx = edx;
        flags = subFlags(edx, other);
        if (static_cast<int32_t>(edx) >= static_cast<int32_t>(other)) {
            r->eax = 0x4000u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041556f, FUN_0041556f, "regs(esi:ptr[0xfe]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
