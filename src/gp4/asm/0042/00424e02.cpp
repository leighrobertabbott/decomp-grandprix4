#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00424e02
void __cdecl FUN_00424e02(gp4::Regs* r) {
    const uint32_t index = r->eax;
    // imul eax, eax, 0x78 (its flags are replaced by the add below)
    const uint32_t offset = index * 0x78u;
    const uint32_t base = 0x0063bd7cu;
    const uint32_t record = offset + base;
    // add eax, 0x63bd7c
    const uint32_t flags = (record < offset ? 1u : 0u)
        | parityFlag(record)
        | (((offset ^ base ^ record) & 0x10u) != 0u ? 0x10u : 0u)
        | (record == 0u ? 0x40u : 0u)
        | ((record & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(offset ^ base) & (offset ^ record) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eax = GP4_FIELD(uint32_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), 0x4c);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424e02, FUN_00424e02, "regs(eax:u32[0..15]) -> (eax, cf, pf, af, zf, sf, of)")
