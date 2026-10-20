#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0040c566
// Eases the surface grip level 0x0063b838 towards a target 0x0063b83c. The surface map cell under the
// car (0x0040c43b at 0,0) selects the target: kinds 1 and 4 (kerb-like) draw it at random (an odd
// draw of 0x00401462 keeps the old target, otherwise a Q14 table cosine of a random angle is
// clamped to +-0x4000 and blended with the kind's base value, capped at 0x10000); any other kind
// takes the base value 0x00622590[kind]. The level then moves by (target - level) * 0x00603854 >> 16.
// pushal/popal restore every register; popal does not restore the flags, which are those of the
// final `add [0x0063b838], eax`.
void __cdecl FUN_0040c566(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0u;
    c.edx = 0u;
    gp4::call_regs(0x0040c43b, c);
    const uint32_t kind = c.eax;

    if ((kind & 0xffu) == 1u || (kind & 0xffu) == 4u) {
        gp4::call_regs(0x00401462, c);
        if ((c.eax & 1u) == 0u) {
            gp4::call_regs(0x00401462, c);
            uint16_t angle = static_cast<uint16_t>((c.eax & 0xffu) << 6);
            angle = static_cast<uint16_t>(static_cast<uint16_t>(0u - angle) + 0x4000u);
            if (static_cast<int16_t>(angle) < 0) {
                angle = static_cast<uint16_t>(0u - angle);
            }
            const uint32_t offset = (static_cast<uint32_t>(angle >> 2)) & 0xfffeu;
            int32_t wave = static_cast<int32_t>(GP4_ARRAY(int16_t, 0x007c5c5c)[offset >> 1]) << 2;
            if (wave < 0) {
                uint32_t magnitude = 0u - static_cast<uint32_t>(wave);
                if (magnitude >= 0x4000u) {
                    magnitude = 0x4000u;
                }
                wave = static_cast<int32_t>(0u - magnitude);
            } else if (static_cast<uint32_t>(wave) >= 0x4000u) {
                wave = 0x4000;
            }
            const uint32_t half = (static_cast<uint32_t>(wave) + 0x4000u) >> 1;
            const int32_t base = GP4_ARRAY(int32_t, 0x00622590)[kind];
            const int32_t span = static_cast<int32_t>(0x10000u - static_cast<uint32_t>(base));
            int32_t blended = static_cast<int32_t>(static_cast<uint32_t>(
                static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(half)) * static_cast<int64_t>(span)) >> 14));
            blended = static_cast<int32_t>(static_cast<uint32_t>(blended) + static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x00622590)[kind]));
            if (blended > 0x10000) {
                blended = 0x10000;
            }
            GP4_GLOBAL(int32_t, 0x0063b83c) = blended;
        }
    } else {
        GP4_GLOBAL(int32_t, 0x0063b83c) = GP4_ARRAY(int32_t, 0x00622590)[kind];
    }

    const uint32_t level = GP4_GLOBAL(uint32_t, 0x0063b838);
    const int32_t gap = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0063b83c) - GP4_GLOBAL(uint32_t, 0x0063b838));
    const uint32_t step = static_cast<uint32_t>(
        static_cast<uint64_t>(static_cast<int64_t>(gap) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854))) >> 16);
    GP4_GLOBAL(uint32_t, 0x0063b838) = level + step;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(level, step);
}
GP4_IMPL(0x0040c566, FUN_0040c566, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+map_0040c43b+tab_00622590+surf_0040c566")
