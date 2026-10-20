#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF excluded) of the 32-bit `inc` result.
static inline uint32_t incFlags32(uint32_t before) {
    const uint32_t result = before + 1u;
    return parityFlag(result)
        | (((before ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (before == 0x7fffffffu ? 0x800u : 0u);
}

// Arithmetic flags of the 32-bit `sub left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Three-way comparison of the clock 0x0066d814 with the stamp 0x0062f930 (-1 = none): no stamp, or a
// stamp before the clock, sets byte 0x0062f8f8 and counts in 0x0062f948 (0x0062f950 counts the no-stamp
// case); a stamp after the clock sets byte 0x0062f8f9 and counts in 0x0062f944; an equal stamp clears byte
// 0x0062f8f9. eax is the difference on the compared paths; the flags are those of the last sub / or / inc
// (inc keeps CF).
// FUNCTION: GP4 0x00445d18
void __cdecl FUN_00445d18(gp4::Regs* r) {
    const uint32_t stamp = GP4_GLOBAL(uint32_t, 0x0062f930);
    uint32_t flags;
    if (stamp == 0xffffffffu) {
        // cmp [stamp], -1 (equal: CF = 0), then two incs which keep CF
        const uint32_t first = GP4_GLOBAL(uint32_t, 0x0062f950);
        GP4_GLOBAL(uint32_t, 0x0062f950) = first + 1u;
        GP4_GLOBAL(uint8_t, 0x0062f8f8) = 0xffu;
        const uint32_t second = GP4_GLOBAL(uint32_t, 0x0062f948);
        GP4_GLOBAL(uint32_t, 0x0062f948) = second + 1u;
        flags = incFlags32(second);
    } else {
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        const uint32_t diff = stamp - clock;
        r->eax = diff;
        if (diff == 0u) {
            GP4_GLOBAL(uint8_t, 0x0062f8f9) = 0u;
            flags = subFlags32(stamp, clock);
        } else if ((diff & 0x80000000u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x0062f8f9) = 0xffu;
            const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062f944);
            GP4_GLOBAL(uint32_t, 0x0062f944) = count + 1u;
            flags = incFlags32(count);
        } else {
            GP4_GLOBAL(uint8_t, 0x0062f8f8) = 0xffu;
            const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062f948);
            GP4_GLOBAL(uint32_t, 0x0062f948) = count + 1u;
            flags = incFlags32(count);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445d18, FUN_00445d18, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
