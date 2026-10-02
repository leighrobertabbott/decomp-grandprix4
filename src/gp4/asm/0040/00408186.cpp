#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00408186
void __cdecl FUN_00408186(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x124);
    // or eax, eax gives CF=OF=AF=0 and ZF/SF/PF from the value;
    // stc (nonzero) or clc (zero) then sets CF to the nonzero test.
    const uint32_t flags = (value != 0u ? 1u : 0u)
        | parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408186, FUN_00408186, "regs(esi:ptr[0x12c]:layout=rec_00408186) -> (eax, cf, pf, af, zf, sf, of)")
