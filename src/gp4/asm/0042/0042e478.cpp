#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0042e478
void __cdecl FUN_0042e478(gp4::Regs* r) {
    // Compare the signed words at +0x90 of the esi and ebx records:
    // eax = +1 when esi.word - ebx.word has no sign (16-bit sub), else -1.
    // The flags are those of "sub ax, word ptr [ebx + 0x90]".
    uint8_t* const first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t left = GP4_FIELD(uint16_t, first, 0x90);
    const uint32_t right = GP4_FIELD(uint16_t, second, 0x90);
    const uint32_t result = (left - right) & 0xffffu;
    const uint32_t flags = parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
    r->eax = (result & 0x8000u) != 0u ? 0xffffffffu : 1u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042e478, FUN_0042e478, "regs(esi:ptr[0x98]:bytes, ebx:ptr[0x98]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
