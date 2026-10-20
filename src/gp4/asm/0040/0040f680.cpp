#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of "neg v" (0 - v).
static inline uint32_t negFlags32(uint32_t v) {
    const uint32_t res = 0u - v;
    uint32_t f = parityFlag(res);
    if (v != 0u) f |= 0x1u;
    if ((v & 0xfu) != 0u) f |= 0x10u;
    if (res == 0u) f |= 0x40u;
    if ((res & 0x80000000u) != 0u) f |= 0x80u;
    if (v == 0x80000000u) f |= 0x800u;
    return f;
}

// FUNCTION: GP4 0x0040f680
void __cdecl FUN_0040f680(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;

    uint32_t value = GP4_FIELD(uint32_t, body, 0x28c);
    if ((int32_t)value < 0) {
        value = 0u - value;
        if (!(value < 0x1000000u)) {
            value = 0x1000000u;
        }
        value = 0u - value;
    } else if (!(value < 0x1000000u)) {
        value = 0x1000000u;
    }
    GP4_FIELD(uint32_t, body, 0xac) = value;
    const uint32_t negated = 0u - value;
    GP4_FIELD(uint32_t, body, 0x158) = negated;
    GP4_FIELD(uint32_t, body, 0x154) = 0u;
    GP4_FIELD(uint32_t, body, 0x15c) = 0u;

    r->eax = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | negFlags32(value);
}
GP4_IMPL(0x0040f680, FUN_0040f680, "regs(esi:ptr[0x294]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
