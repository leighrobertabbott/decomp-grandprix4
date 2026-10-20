#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit add.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043c235
// Rate-limited refresh of the six blended values (0x0043c083): while the "armed" byte [0x63b860]
// is 0xff and a due time [0x63b8e4] is set, the refresh is skipped until the race clock
// [0x66d814] reaches it (the sign of clock - due time decides; eax and the flags of that add
// stay). Otherwise [0x62278c] = word [0x66d89c], 0x0043c083 runs (pushal/popal) and the next due
// time becomes clock + 3000 (the flags are those of that add), the armed byte 0xff. eax is the
// zero-extended word in that case.
void __cdecl FUN_0043c235(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0063b860) == 0xffu && GP4_GLOBAL(uint32_t, 0x0063b8e4) != 0u) {
        const uint32_t negatedDue = 0u - GP4_GLOBAL(uint32_t, 0x0063b8e4);
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        const uint32_t difference = negatedDue + clock;
        if ((difference & 0x80000000u) != 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | addFlags32(negatedDue, clock);
            return;
        }
    }

    const uint32_t word = GP4_GLOBAL(uint16_t, 0x0066d89c);
    GP4_GLOBAL(uint32_t, 0x0062278c) = word;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = word;
    gp4::call_regs(0x0043c083, c);

    const uint32_t clockNow = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_GLOBAL(uint32_t, 0x0063b8e4) = clockNow;
    GP4_GLOBAL(uint32_t, 0x0063b8e4) += 0xbb8u;
    GP4_GLOBAL(uint8_t, 0x0063b860) = 0xffu;

    r->eax = c.eax;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(clockNow, 0xbb8u);
}
GP4_IMPL(0x0043c235, FUN_0043c235, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_0040c0ae+lim_00937dd0+state_0043c1ba+state_0040b99a+map_0040c43b+state_0040bb26+clock_0066d814+state_0043c235")
