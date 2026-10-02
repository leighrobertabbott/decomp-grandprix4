#include <gp4/gp4.h>

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004082cd
void __cdecl FUN_004082cd(gp4::Regs* r) {
    const uint32_t selector = r->ebx;
    const uint32_t index = r->edx;
    const uint8_t* base = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    // cmp ebx, -1; je -> sign-extended byte at +0x210, else 0
    int32_t value = 0;
    if (selector == 0xffffffffu) {
        value = static_cast<int8_t>(base[index + 0x210u]);
    }
    r->eax = static_cast<uint32_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags32(selector, 0xffffffffu);
}
GP4_IMPL(0x004082cd, FUN_004082cd, "regs(ebx:i32[-1..1], edx:i32[0..11], esi:ptr[0x228]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
