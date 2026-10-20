#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Arithmetic flags of the 8-bit `cmp value, 0`.
static inline uint32_t cmpZeroFlags8(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// One step of the track-surface warm-up: unless the byte [0x006f299d] is set, pushal; when the byte
// [0x006f29a0] is set and the target level [0x00622584] is not -1 re-seeds the surface through
// 0x0040c2c0, then advances the segments with 0x0040bfcf and refreshes the ring with 0x0040c0ae; popal.
// Registers are preserved; the flags are those of `cmp [0x6f299d], 0` when the byte is set, otherwise
// those 0x0040c0ae leaves.
// FUNCTION: GP4 0x0040c1d6
void __cdecl FUN_0040c1d6(gp4::Regs* r) {
    const uint32_t skip = GP4_GLOBAL(uint8_t, 0x006f299d);
    if (skip != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | cmpZeroFlags8(skip);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = (c.eflags & ~0x8d5u) | 0x44u;

    if (GP4_GLOBAL(uint8_t, 0x006f29a0) != 0u && GP4_GLOBAL(int32_t, 0x00622584) != -1) {
        gp4::call_regs(0x0040c2c0, c);
    }
    gp4::call_regs(0x0040bfcf, c);
    gp4::call_regs(0x0040c0ae, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0040c1d6, FUN_0040c1d6, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+map_0040c43b+ring_0040c0ae+state_0040bec7+state_0040bd5f+state_0040bce6")
