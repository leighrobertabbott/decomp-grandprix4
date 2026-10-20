#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags (CF PF AF ZF SF OF) of the 32-bit "cmp/sub left, right".
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags (CF PF AF ZF SF OF) of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The tests run on the live registers: the original leaves eax/edx modified wherever it stops, and
// the flags of its last arithmetic instruction (stc/clc only set CF).
static inline bool overlaps(uint32_t& eax, uint32_t& edx, uint32_t& flags) {
    const uint32_t bound = GP4_GLOBAL(uint32_t, 0x00891500);
    flags = subFlags32(eax, bound);
    if (eax >= bound) return false;
    flags = subFlags32(edx, bound);
    if (edx < bound) return false;
    const uint32_t first = GP4_GLOBAL(uint32_t, 0x00866ef0);
    flags = subFlags32(eax, first);
    if (eax >= first) return false;

    flags = addFlags32(eax, 0x580u);
    eax += 0x580u;
    flags = subFlags32(eax, first);
    if (!(eax < first)) {
        const uint32_t second = GP4_GLOBAL(uint32_t, 0x00866ed0);
        flags = subFlags32(edx, second);
        if (edx < second) return false;
        flags = subFlags32(edx, 0x580u);
        edx -= 0x580u;
        flags = subFlags32(edx, second);
        return edx < second;
    }

    flags = subFlags32(eax, 0x580u);
    eax -= 0x580u;
    const uint32_t third = GP4_GLOBAL(uint32_t, 0x00866ef4);
    flags = subFlags32(eax, third);
    if (eax < third) return false;
    flags = subFlags32(eax, 0x580u);
    eax -= 0x580u;
    flags = subFlags32(eax, third);
    if (eax >= third) return false;
    const uint32_t fourth = GP4_GLOBAL(uint32_t, 0x00866ed4);
    flags = subFlags32(edx, fourth);
    if (edx >= fourth) return false;
    flags = addFlags32(edx, 0x580u);
    edx += 0x580u;
    flags = subFlags32(edx, fourth);
    return !(edx < fourth);
}

// Interval-overlap test of the address pair (eax, edx) against DAT_00891500 and the two intervals
// [DAT_00866ef4, DAT_00866ef0) / [DAT_00866ed0, DAT_00866ed4) widened by 0x580. CF = 1 (stc) when it
// holds, 0 (clc) otherwise; the other flags are those of the last arithmetic instruction.
// FUNCTION: GP4 0x0042b10d
void __cdecl FUN_0042b10d(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags = 0;
    const bool hit = overlaps(eax, edx, flags);

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | (hit ? 1u : 0u);
}
GP4_IMPL(0x0042b10d, FUN_0042b10d, "regs(eax:rec[0x8943a0,0x160,0..11], edx:rec[0x8943a0,0x160,0..11]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=win_0042b10d")
