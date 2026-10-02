#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00408170
void __cdecl FUN_00408170(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t link = GP4_FIELD(uint16_t, const_cast<uint8_t*>(record), 0x1bc);
    if (link == 0xffffu) {
        // cmp ax, -1 equal (ZF|PF), then clc
        r->eax = link;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
    } else {
        // add eax, 0x66fef0 then stc
        const uint32_t base = 0x0066fef0u;
        const uint32_t result = link + base;
        const uint32_t flags = 1u
            | parityFlag(result)
            | (((link ^ base ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | ((~(link ^ base) & (link ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
        r->eax = result;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
    }
}
GP4_IMPL(0x00408170, FUN_00408170, "regs(esi:ptr[0x1c4]:layout=rec_00408170) -> (eax, cf, pf, af, zf, sf, of)")
