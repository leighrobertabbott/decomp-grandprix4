#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004069e6
void __cdecl FUN_004069e6(gp4::Regs* r) {
    // Step edi back by one 0x160-byte record; below the table start 0x8943a0 wrap to
    // [0x866ecc] - 0x160. The flags are those of the cmp, or of the wrap's sub.
    uint32_t edi = r->edi - 0x160u;
    uint32_t flags = subFlags(edi, 0x008943a0u);
    if (edi < 0x008943a0u) {
        const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ecc);
        edi = end - 0x160u;
        flags = subFlags(end, 0x160u);
    }
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004069e6, FUN_004069e6, "regs(edi:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
