#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043679c
// pushal ... popal routine. Clears the three OR words (+0xc, +0x14, +0x20) of the two 0x28-byte
// collision objects at 0x00629e0c and 0x00629e34, lets 0x0043659c compare them in both orders and
// copies each object's three results into the "previous" words (+0x10, +0x18, +0x24 relative to the
// object: 0x629e1c/24/30 and 0x629e44/4c/58) when its first word is nonzero. Returns
// eax = first | second and carry set (stc) when either collided; the other flags are those of the OR.
void __cdecl FUN_0043679c(gp4::Regs* r) {
    uint8_t* first = GP4_ARRAY(uint8_t, 0x00629e0c);
    uint8_t* second = GP4_ARRAY(uint8_t, 0x00629e34);
    GP4_FIELD(uint32_t, second, 0xc) = 0;
    GP4_FIELD(uint32_t, first, 0xc) = 0;
    GP4_FIELD(uint32_t, second, 0x14) = 0;
    GP4_FIELD(uint32_t, first, 0x14) = 0;
    GP4_FIELD(uint32_t, second, 0x20) = 0;
    GP4_FIELD(uint32_t, first, 0x20) = 0;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00629e34u;
    c.esi = 0x00629e0cu;
    gp4::call_regs(0x0043659c, c);
    c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00629e0cu;
    c.esi = 0x00629e34u;
    gp4::call_regs(0x0043659c, c);

    if (GP4_GLOBAL(uint32_t, 0x00629e18) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00629e1c) = GP4_GLOBAL(uint32_t, 0x00629e18);
        GP4_GLOBAL(uint32_t, 0x00629e24) = GP4_GLOBAL(uint32_t, 0x00629e20);
        GP4_GLOBAL(uint32_t, 0x00629e30) = GP4_GLOBAL(uint32_t, 0x00629e2c);
    }
    if (GP4_GLOBAL(uint32_t, 0x00629e40) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00629e44) = GP4_GLOBAL(uint32_t, 0x00629e40);
        GP4_GLOBAL(uint32_t, 0x00629e4c) = GP4_GLOBAL(uint32_t, 0x00629e48);
        GP4_GLOBAL(uint32_t, 0x00629e58) = GP4_GLOBAL(uint32_t, 0x00629e54);
    }

    const uint32_t combined = GP4_GLOBAL(uint32_t, 0x00629e18) | GP4_GLOBAL(uint32_t, 0x00629e40);
    r->eax = combined;
    // "or" clears CF/OF/AF; stc/clc then set CF
    const uint32_t flags = parityFlag(combined)
        | (combined == 0u ? 0x40u : 0u)
        | ((combined & 0x80000000u) != 0u ? 0x80u : 0u)
        | (combined != 0u ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043679c, FUN_0043679c, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
