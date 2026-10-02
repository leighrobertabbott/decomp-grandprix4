#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004340b1
void __cdecl FUN_004340b1(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t field = r->edx;
    // imul eax, eax, 0x1c (its flags are replaced by the add below)
    const uint32_t offset = index * 0x1cu;
    const uint32_t base = 0x0062ffacu;
    const uint32_t record = offset + base;
    // add eax, 0x62ffac
    const uint32_t flags = (record < offset ? 1u : 0u)
        | parityFlag(record)
        | (((offset ^ base ^ record) & 0x10u) != 0u ? 0x10u : 0u)
        | (record == 0u ? 0x40u : 0u)
        | ((record & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(offset ^ base) & (offset ^ record) & 0x80000000u) != 0u ? 0x800u : 0u);
    // mov al, [edx + eax] keeps the upper 24 bits of the record address
    const uint8_t byte = GP4_FIELD(uint8_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), field);
    r->eax = (record & 0xffffff00u) | byte;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004340b1, FUN_004340b1, "regs(eax:u32[0..15], edx:u32[0..27]) -> (eax, cf, pf, af, zf, sf, of)")
