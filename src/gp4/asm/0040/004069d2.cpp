#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004069d2
void __cdecl FUN_004069d2(gp4::Regs* r) {
    // edi += 0x160; wrap to the table start 0x8943a0 when it reaches [0x866ecc] (unsigned).
    // The wrap is a plain mov, so every flag is that of the cmp.
    const uint32_t left = r->edi + 0x160u;
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x00866ecc);
    const uint32_t result = left - right;
    const uint32_t flags = parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->edi = (left < right) ? left : 0x008943a0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004069d2, FUN_004069d2, "regs(edi:u32) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
