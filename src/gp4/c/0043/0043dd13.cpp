#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// flags of cmp / sub left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// flags of a 32-bit test / or: CF = OF = AF = 0
static inline uint32_t testFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of a byte test: SF is bit 7 of the byte result
static inline uint32_t testFlags8(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// FUNCTION: GP4 0x0043dd13
void __cdecl FUN_0043dd13(gp4::Regs* r) {
    // pushal ... popal: every register comes back, only the flags of the last flag writer remain
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0062ac48);
    uint32_t flags = testFlags(remaining);
    if (remaining == 0u) {
        GP4_GLOBAL(uint32_t, 0x00603ed0) = 0u;
    } else {
        for (;;) {
            const uint32_t marks = GP4_FIELD(uint8_t, record, 0x51) & 0x80u;
            flags = testFlags8(marks);
            if (marks != 0u) {
                // byte +0xb6 selects a 0x40-byte entry of the table at 0x00626fb8
                const uint32_t id = GP4_FIELD(uint8_t, record, 0xb6);
                flags = testFlags(id);
                if (id == 0u) {
                    GP4_GLOBAL(uint32_t, 0x00603ed0) = 0u;
                } else {
                    const uint32_t offset = id * 0x40u;
                    flags = addFlags(offset, 0x00626fb8u);
                    GP4_GLOBAL(uint32_t, 0x00603ed0) = GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x00626fb8), offset);
                }
                break;
            }

            // step to the next 0x160-byte record, wrapping at the array bounds
            const uint32_t before = reinterpret_cast<uint32_t>(record);
            uint32_t next = before + 0x160u;
            flags = addFlags(before, 0x160u);
            if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
                const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ef8);
                flags = subFlags(next, limit);
                if (next >= limit) {
                    flags = subFlags(next, 0x160u);
                    next -= 0x160u;
                }
            } else {
                const uint32_t low = GP4_GLOBAL(uint32_t, 0x00866ecc);
                flags = subFlags(next, low);
                if (next >= low) {
                    const uint32_t high = GP4_GLOBAL(uint32_t, 0x00891500);
                    flags = subFlags(next, high);
                    if (next < high) {
                        const uint32_t shift = GP4_GLOBAL(uint32_t, 0x00866ec4);
                        flags = subFlags(next, shift);
                        next -= shift;
                    } else {
                        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00866ef8);
                        flags = subFlags(next, limit);
                        if (next >= limit) {
                            flags = subFlags(limit, 0x160u);
                            next = limit - 0x160u;
                        }
                    }
                }
            }
            record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(next));
            --remaining;
            if (remaining == 0u) {
                break;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043dd13, FUN_0043dd13, "regs(edi:rec[0x8943a0,0x160,0..29]) -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
