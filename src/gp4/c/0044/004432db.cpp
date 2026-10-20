#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a 16-bit INC (CF untouched, so it is left out).
static inline uint32_t incFlags16(uint16_t before) {
    const uint16_t result = static_cast<uint16_t>(before + 1u);
    return byteParity(result)
        | ((before & 0xfu) == 0xfu ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | (before == 0x7fffu ? 0x800u : 0u);
}

// pushal ... popal: every register is restored; the flags are those of the last flag writer with CF forced
// (stc after the increment, clc on every early exit).
// Advances the word counter [0x0066d876] (below the limit [0x0066d8ec]) when 0x00443278 finds a carry and
// 0x0044329f does not for the code byte [0x0062ad70].
// FUNCTION: GP4 0x004432db
void __cdecl FUN_004432db(gp4::Regs* r) {
    const uint16_t counter = GP4_GLOBAL(uint16_t, 0x0066d876);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t flags = cmpFlags32(counter, limit);
    uint32_t carry = 0u;

    if (static_cast<int32_t>(counter) < static_cast<int32_t>(limit)) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.edx = counter;
        gp4::call_regs(0x00443278, c);
        flags = c.eflags & 0x8d5u;
        if ((c.eflags & 1u) != 0u) {
            c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0062ad70);
            gp4::call_regs(0x0044329f, c);
            flags = c.eflags & 0x8d5u;
            if ((c.eflags & 1u) == 0u) {
                const uint16_t before = GP4_GLOBAL(uint16_t, 0x0066d876);
                GP4_GLOBAL(uint16_t, 0x0066d876) = static_cast<uint16_t>(before + 1u);
                flags = incFlags16(before) | (c.eflags & 1u);
                carry = 1u;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x004432db, FUN_004432db, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+flags_0066d909+keys_0044329f")
