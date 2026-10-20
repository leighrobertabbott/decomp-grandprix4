#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Picks the best tyre column of a level group. edx is the first column of the group (an entry of the level table
// 0x006f2cc4), eax the car row. Starting at column [0x6f2c6a + edx] and ending before [0x6f2c96 + edx], every column
// is a candidate when the group's byte [0x6f2c3e + edx] is at least 2 (signed) or its availability byte
// [0x6f2cdc + 8 * mode + column offset] is 0xff, where mode is the result of 0x0051fb4f. A candidate wins when its
// value in the 0x2c-dword row of the car (0x66ec60 + row * 0xb0) is below the best so far by more than 0x200000
// (unsigned; the best starts at 0xffffffff). edx returns the winning column (or the first one when none wins);
// all other registers are restored; the flags are those of the loop's last `cmp ecx, ebx`.
// FUNCTION: GP4 0x004411f7
void __cdecl FUN_004411f7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0051fb4f, c);

    const uint32_t group = r->edx;
    uint32_t availability = c.eax * 8u;
    const uint32_t rowBase = (r->eax * 0x2cu) << 2;
    uint32_t column = GP4_ARRAY(const uint8_t, 0x006f2c6a)[group];
    const uint32_t end = GP4_ARRAY(const uint8_t, 0x006f2c96)[group];
    uint32_t best = column;
    uint32_t bestValue = 0xffffffffu;
    uint32_t flags;

    do {
        const bool open = static_cast<int8_t>(GP4_ARRAY(const uint8_t, 0x006f2c3e)[group]) >= 2
            || GP4_ARRAY(const uint8_t, 0x006f2cdc)[availability] == 0xffu;
        if (open) {
            const uint32_t value = GP4_ARRAY(const uint32_t, 0x0066ec60)[(rowBase >> 2) + column];
            if (bestValue > value && bestValue > value + 0x200000u) {
                best = column;
                bestValue = value;
            }
        }
        availability += 1u;
        column += 1u;
        flags = cmpFlags32(column, end);
    } while (static_cast<int32_t>(column) < static_cast<int32_t>(end));

    r->edx = best;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004411f7, FUN_004411f7, "regs(eax:u32[0..21], edx:u32[0..37]) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=scale_0066ec60+state_004320f2")
