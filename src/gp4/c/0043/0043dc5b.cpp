#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit "cmp left, right" (CF included).
static inline uint32_t compareFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// flags of cmp / sub left, right (32-bit)
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// pushal ... popal: every register comes back; only the flags of the closing compare remain.
// Walks [0x0062ac48] ring records from edi (one pass at least; the forward advance is the lap/pit
// rule of 0x0043dd13) and keeps two signed minima over the records:
//   bp = min(8 * [0x006077da] - word[rec+0x16])   (0x0043d2c4 stores [0x006077da])
//   si = min(8 * [0x006077d8] + word[rec+0x16])   (0x0043d27c stores [0x006077d8])
// Then bp = max(bp, si) (signed) and CF = (bp >= 0x180 unsigned) via jae/stc, clc, with the other
// flags those of "cmp bp, 0x180".
// FUNCTION: GP4 0x0043dc5b
void __cdecl FUN_0043dc5b(gp4::Regs* r) {
    gp4::Regs state = *r;
    uint16_t bp = 0x7fffu;
    uint16_t si = 0x7fffu;
    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0062ac48);
    for (;;) {
        gp4::call_regs(0x0043d2c4, state);
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(state.edi));
        const uint16_t high = static_cast<uint16_t>(
            static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x006077da) << 3) - GP4_FIELD(uint16_t, record, 0x16));
        if (static_cast<int16_t>(bp) > static_cast<int16_t>(high)) {
            bp = high;
        }
        gp4::call_regs(0x0043d27c, state);
        const uint16_t low = static_cast<uint16_t>(
            static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x006077d8) << 3) + GP4_FIELD(uint16_t, record, 0x16));
        if (static_cast<int16_t>(si) > static_cast<int16_t>(low)) {
            si = low;
        }

        uint32_t next = state.edi + 0x160u;
        if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0u) {
            if (next >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                next -= 0x160u;
            }
        } else {
            if (next >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                if (next < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    next -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                } else if (next >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    next = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                }
            }
        }
        state.edi = next;

        remaining -= 1u;
        if (static_cast<int32_t>(remaining) <= 0) {
            break;
        }
    }

    if (static_cast<int16_t>(bp) < static_cast<int16_t>(si)) {
        bp = si;
    }
    const uint32_t flags = compareFlags16(bp, 0x180u);
    const uint32_t carry = bp >= 0x180u ? 1u : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x0043dc5b, FUN_0043dc5b, "regs(edi:rec[0x8943a0,0x160,0..23]) -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+ring_0043f40d")
