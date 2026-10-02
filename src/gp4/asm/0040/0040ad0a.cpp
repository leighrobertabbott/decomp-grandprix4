#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0040ad0a
void __cdecl FUN_0040ad0a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint8_t value = GP4_FIELD(uint8_t, record, 0x51) | 4u;
    GP4_FIELD(uint8_t, record, 0x51) = value;
    GP4_FIELD(uint8_t, record, 0x68) = static_cast<uint8_t>(r->edx);
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag(value) | (value & 0x80u);
}
GP4_IMPL(0x0040ad0a, FUN_0040ad0a, "regs(ebx:ptr[0x70]:bytes, edx:u32) -> (cf, pf, af, zf, sf, of)")
