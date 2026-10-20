#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Builds the opcode flag byte in al: the top five bits of the mask byte 0x007c2ead, bit 1 when bit 7 of 0x0066d87c
// is set, bit 2 when the byte 0x0095404e is 0xff. eax keeps its upper 24 bits; the flags are those of the last
// cmp (or of the `or al, 4` when that bit was added).
// FUNCTION: GP4 0x00445218
void __cdecl FUN_00445218(gp4::Regs* r) {
    uint32_t flag = GP4_GLOBAL(uint8_t, 0x007c2ead) & 0xf8u;
    if ((GP4_GLOBAL(uint8_t, 0x0066d87c) & 0x80u) != 0u) {
        flag |= 2u;
    }
    const uint32_t marker = GP4_GLOBAL(uint8_t, 0x0095404e);
    uint32_t flags = cmpFlags8(marker, 0xffu);
    if (marker == 0xffu) {
        flag |= 4u;
        flags = logicFlags8(flag);
    }
    r->eax = (r->eax & 0xffffff00u) | flag;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445218, FUN_00445218, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
