#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of `test byte, 2` (result 0 or 2): CF = OF = AF = 0.
static inline uint32_t testFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u);
}

// FUNCTION: GP4 0x00414f29
// Raises eax (signed) to a lower bound chosen by the body's flag bits: when bit 1 of +0x232 is set,
// the bound is DAT_006247dc if bit 1 of +0xca is set, else DAT_006247d8 if bit 1 of +0xcb is set,
// else DAT_006247d4 if bit 1 of +0x273 is set; with no bound eax is unchanged. The flags are those
// of the last compare or test executed.
void __cdecl FUN_00414f29(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;
    const int32_t value = static_cast<int32_t>(r->eax);

    uint32_t flags;
    int32_t bound;
    bool compared = true;
    uint32_t tested = GP4_FIELD(uint8_t, body, 0x232) & 2u;
    if (tested == 0u) {
        compared = false;
        flags = testFlags(tested);
        bound = value;
    } else if ((GP4_FIELD(uint8_t, body, 0xca) & 2u) != 0u) {
        bound = GP4_GLOBAL(int32_t, 0x006247dc);
    } else if ((GP4_FIELD(uint8_t, body, 0xcb) & 2u) != 0u) {
        bound = GP4_GLOBAL(int32_t, 0x006247d8);
    } else {
        tested = GP4_FIELD(uint8_t, body, 0x273) & 2u;
        if (tested == 0u) {
            compared = false;
            flags = testFlags(tested);
            bound = value;
        } else {
            bound = GP4_GLOBAL(int32_t, 0x006247d4);
        }
    }

    if (compared) {
        flags = cmpFlags32(static_cast<uint32_t>(value), static_cast<uint32_t>(bound));
    }
    r->eax = static_cast<uint32_t>(value < bound ? bound : value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414f29, FUN_00414f29, "regs(eax:i32, esi:ptr[0x27b]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
