#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00414f79
void __cdecl FUN_00414f79(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t left = GP4_FIELD(uint8_t, record, 0x6d);
    const uint32_t right = 0x40u;
    const uint32_t result = (left - right) & 0xffu;
    // cmp al, 0x40 (the movs leave the flags alone)
    const uint32_t flags = (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
    r->eax = static_cast<int8_t>(left) < 0x40 ? 0u : 0x4000u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414f79, FUN_00414f79, "regs(esi:ptr[0x75]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
