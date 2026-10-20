#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp left, right` (dword).
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Calls the 0x0040b629 record reset (edi = record) for every 0x160-byte record of the first
// range (from 0x00894240 up to and including the end pointer 0x00866ecc) and of the second
// range (from the record before 0x00891500 up to and including 0x00866ef8).
// A pushal/popal routine: only the flags of the last cmp survive.
// FUNCTION: GP4 0x0040b652
void __cdecl FUN_0040b652(gp4::Regs* r) {
    uint32_t edi = 0x00894240u;
    do {
        gp4::Regs callee = *r;
        callee.edi = edi;
        gp4::call_regs(0x0040b629, callee);
        edi += 0x160u;
    } while (edi <= GP4_GLOBAL(uint32_t, 0x00866ecc));

    edi = GP4_GLOBAL(uint32_t, 0x00891500) - 0x160u;
    uint32_t flags;
    do {
        gp4::Regs callee = *r;
        callee.edi = edi;
        gp4::call_regs(0x0040b629, callee);
        edi += 0x160u;
        flags = subFlags(edi, GP4_GLOBAL(uint32_t, 0x00866ef8));
    } while (edi <= GP4_GLOBAL(uint32_t, 0x00866ef8));

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b652, FUN_0040b652, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=records_008943a0")
