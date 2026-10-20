#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right (sub is the same)
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// flags of or reg,reg (logic: CF=OF=AF=0)
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of shr reg,1 (CF = bit 0, OF = old bit 31, AF = 0)
static inline uint32_t shr1Flags(uint32_t value) {
    const uint32_t result = value >> 1;
    return (value & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043eb10
// Steps the ring-buffer cursor 0x00602d84 (record stride 0x160) half of
// 0x0062ac48 times, wrapping at the ring limits, unless 0x00603ed0 is set.
// pushal/popal wrapper: only the flags of the last flag writer escape.
void __cdecl FUN_0043eb10(gp4::Regs* r) {
    uint32_t cursor = GP4_GLOBAL(uint32_t, 0x00603ed0);
    uint32_t flags = logicFlags(cursor);
    if (cursor == 0u) {
        cursor = GP4_GLOBAL(uint32_t, 0x00626990);
        const uint32_t total = GP4_GLOBAL(uint32_t, 0x0062ac48);
        uint32_t count = total >> 1;
        flags = shr1Flags(total);
        while (count != 0u) {
            cursor += 0x160u;
            if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0) {
                const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ef8);
                flags = subFlags(cursor, limit);
                if (cursor >= limit) {
                    flags = subFlags(cursor, 0x160u);
                    cursor -= 0x160u;
                }
            } else {
                const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866ecc);
                flags = subFlags(cursor, low);
                if (cursor >= low) {
                    const uint32_t high = GP4_GLOBAL(uint32_t, 0x00891500);
                    flags = subFlags(cursor, high);
                    if (cursor < high) {
                        const uint32_t delta = GP4_GLOBAL(uint32_t, 0x00866ec4);
                        flags = subFlags(cursor, delta);
                        cursor -= delta;
                    } else {
                        const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ef8);
                        flags = subFlags(cursor, end);
                        if (cursor >= end) {
                            const uint32_t reload = GP4_GLOBAL(uint32_t, 0x00866ef8);
                            flags = subFlags(reload, 0x160u);
                            cursor = reload - 0x160u;
                        }
                    }
                }
            }
            --count;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00602d84) = cursor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043eb10, FUN_0043eb10, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0043eb10")
