#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00409ac2
void __cdecl FUN_00409ac2(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = GP4_FIELD(uint32_t, record, 0x40) & 0x3c0u;
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag(value) | (value == 0u ? 0x40u : 0u);
}
GP4_IMPL(0x00409ac2, FUN_00409ac2, "regs(esi:ptr[0x48]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
