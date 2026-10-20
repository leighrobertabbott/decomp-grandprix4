#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Arms the 24-hour deadline: [0x0066d83c] = clock [0x0066d814] + 86400000 (24 h in milliseconds) and the
// word [0x0066d898] is cleared. The flags are those of the 32-bit add.
// FUNCTION: GP4 0x00442cb4
void __cdecl FUN_00442cb4(gp4::Regs* r) {
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_GLOBAL(uint32_t, 0x0066d83c) = clock + 0x05265c00u;
    GP4_GLOBAL(uint16_t, 0x0066d898) = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(clock, 0x05265c00u);
}
GP4_IMPL(0x00442cb4, FUN_00442cb4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
