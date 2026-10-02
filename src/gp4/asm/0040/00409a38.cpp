#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00409a38
void __cdecl FUN_00409a38(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t left = GP4_FIELD(uint16_t, record, 0xea);
    const uint32_t right = 0x20u;
    const uint32_t result = (left - right) & 0xffffu;
    uint32_t flags;
    if (left == right) {
        // cmp equal: ZF|PF, then stc sets CF
        flags = 0x45u;
    } else {
        // 16-bit cmp flags (CF cleared by clc)
        flags = parityFlag(result)
            | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | ((result & 0x8000u) != 0u ? 0x80u : 0u)
            | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409a38, FUN_00409a38, "regs(esi:ptr[0xf2]:layout=rec_00409a38) -> (cf, pf, af, zf, sf, of)")
