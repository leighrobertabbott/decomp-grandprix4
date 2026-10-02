#include <gp4/gp4.h>

static inline uint32_t logicalFlags8(uint8_t value) {
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0042283b
void __cdecl FUN_0042283b(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);
    body[0x200] = 0;
    body[0xcd] &= 0x9fu;
    body[0x162] |= 2u;
    body[0x203] |= 0x40u;
    body[0x21d] |= 0x20u;
    body[0x2bb] = 0;
    r->eflags = (r->eflags & ~0x8d5u) | logicalFlags8(body[0x21d]);
}
GP4_IMPL(0x0042283b, FUN_0042283b, "regs(esi:ptr[0x2c3]:bytes) -> (cf, pf, af, zf, sf, of)")
