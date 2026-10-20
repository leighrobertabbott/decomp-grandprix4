#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0041462f
void __cdecl FUN_0041462f(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    // mov ax, [esi+0x90] ; test ax, ax ; neg ax if negative: the 16-bit magnitude
    uint32_t magnitude = GP4_FIELD(uint16_t, record, 0x90);
    if ((magnitude & 0x8000u) != 0u) {
        magnitude = (0u - magnitude) & 0xffffu;
    }
    // cmp ax, [esi+0xb0]: 16-bit flags, then stc (not less) / clc (less) rewrites CF
    const uint32_t limit = GP4_FIELD(uint16_t, record, 0xb0);
    const uint32_t result = (magnitude - limit) & 0xffffu;
    const uint32_t sign = (result >> 15) & 1u;
    const uint32_t overflow = (((magnitude ^ limit) & (magnitude ^ result)) >> 15) & 1u;
    const uint32_t flags = (sign == overflow ? 1u : 0u)
        | parityFlag(result)
        | (((magnitude ^ limit ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (sign << 7)
        | (overflow << 11);
    r->eax = (r->eax & 0xffff0000u) | magnitude;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041462f, FUN_0041462f, "regs(eax:u32, esi:ptr[0xb8]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
