#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043bd97
// pushal wrapper that rebuilds the per-driver setup: the code bytes (0x0043ba31), the six blended
// values with [0x62278c] = 0 (0x0043c083), the mean [0x6227a0] of [0x622798] and [0x62279c], the
// level byte [0x601fa8] = (signed al of 0x0043ca8b >= 2), then 0x0043bc00, 0x0043bcb8, 0x0043bcd4,
// 0x0043bd63 and 0x0043be59. Every register comes back (popal); the flags are what the last
// callee leaves.
void __cdecl FUN_0043bd97(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0043ba31, c);
    GP4_GLOBAL(uint32_t, 0x0062278c) = 0u;
    gp4::call_regs(0x0043c083, c);

    c.eax = (GP4_GLOBAL(uint32_t, 0x00622798) + GP4_GLOBAL(uint32_t, 0x0062279c)) >> 1;
    GP4_GLOBAL(uint32_t, 0x006227a0) = c.eax;
    gp4::call_regs(0x0043ca8b, c);

    c.eflags = (c.eflags & ~0x8d5u) | cmpFlags8(c.eax, 2u);
    c.eax = (static_cast<int8_t>(c.eax) >= 2) ? 1u : 0u;
    GP4_GLOBAL(uint8_t, 0x00601fa8) = static_cast<uint8_t>(c.eax);

    gp4::call_regs(0x0043bc00, c);
    gp4::call_regs(0x0043bcb8, c);
    gp4::call_regs(0x0043bcd4, c);
    gp4::call_regs(0x0043bd63, c);
    gp4::call_regs(0x0043be59, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043bd97, FUN_0043bd97, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=trk_007ad894+circ_007adb50+codes_0043ba12+tbl_0066d94c+adj_0066d802+state_0043c1ba+state_0043c31b+ring_0040c0ae+lim_00937dd0+state_0040b99a+map_0040c43b+state_0040bb26+state_0043bd97")
