#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// Flags of an 8-bit `or byte [mem], imm` (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `or reg, reg` (CF = OF = AF = 0).
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result >> 31) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `left - right` (cmp / sub).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00445423
// Receives one packet. With the link up ([0x007aafd8] != 0xff) the service 0x4c (0x0051f0b8; carry set
// means success) fills the packet buffer [0x007a97ac] = length + 1, bytes at 0x007a97b1; the payload
// (length bytes, at most 0xfff, none when the length field is 0 or 1) is copied to the dispatch buffer
// 0x0062c19a and its size stored in [0x0062d1a4] (cleared first). Link down or a failed service marks the
// link down (0x007aafd8 = 0xff) and raises bit 5 of 0x008914e0 and bit 4 of 0x007c2ead. pushal / popal
// keep every register; the flags are those of the last flag writer on the path taken.
void __cdecl FUN_00445423(gp4::Regs* r) {
    uint32_t flags;
    bool failed = false;

    if (GP4_GLOBAL(uint8_t, 0x007aafd8) == 0xffu) {
        failed = true;
    } else {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x0051f0b8, c);
        failed = (c.eflags & gp4::CF) == 0u;
    }

    if (failed) {
        GP4_GLOBAL(uint8_t, 0x007aafd8) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x008914e0) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008914e0) | 0x20u);
        const uint8_t marked = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) | 0x10u);
        GP4_GLOBAL(uint8_t, 0x007c2ead) = marked;
        flags = logicFlags8(marked);
    } else {
        GP4_GLOBAL(uint32_t, 0x0062d1a4) = 0u;
        const uint32_t length = GP4_GLOBAL(uint32_t, 0x007a97ac);
        if (length > 0x1000u) {
            flags = subFlags32(length, 0x1000u);
        } else if (length - 1u == 0u) {
            flags = subFlags32(length, 1u);
        } else if (static_cast<int32_t>(length - 1u) < 0) {
            flags = logicFlags32(length - 1u);
        } else {
            const uint32_t count = length - 1u;
            flags = addFlags32(0x007a97b0u, 1u);
            GP4_GLOBAL(uint32_t, 0x0062d1a4) = count;
            const uint8_t* from = GP4_ARRAY(const uint8_t, 0x007a97b1);
            uint8_t* to = GP4_ARRAY(uint8_t, 0x0062c19a);
            for (uint32_t i = 0; i != count; ++i) {
                to[i] = from[i];
            }
        }
    }

    r->eflags = (r->eflags & ~kArithFlags) | (flags & kArithFlags);
}
GP4_IMPL(0x00445423, FUN_00445423, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recv_00445423")
