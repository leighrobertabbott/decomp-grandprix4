#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041902e
// Carry set when (cl + 1) equals one of the three bytes at [esi+0x2b8..0x2ba]; ecx is preserved.
// The other flags are those of the last compare executed (clc/stc only touch CF).
void __cdecl FUN_0041902e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = (r->ecx + 1u) & 0xffu;
    uint32_t flags = cmpFlags8(value, GP4_FIELD(uint8_t, record, 0x2b8));
    bool found = (flags & 0x40u) != 0u;
    if (!found) {
        flags = cmpFlags8(value, GP4_FIELD(uint8_t, record, 0x2b9));
        found = (flags & 0x40u) != 0u;
    }
    if (!found) {
        flags = cmpFlags8(value, GP4_FIELD(uint8_t, record, 0x2ba));
        found = (flags & 0x40u) != 0u;
    }
    flags = (flags & ~1u) | (found ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041902e, FUN_0041902e, "regs(ecx:u32, esi:ptr[0x2c2]:bytes) -> (cf, pf, af, zf, sf, of)")
