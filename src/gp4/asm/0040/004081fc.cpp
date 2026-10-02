#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004081fc
void __cdecl FUN_004081fc(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t left = record[0xe4];
    const uint32_t right = 4u;
    uint32_t flags;
    if (left == right) {
        // cmp leaves ZF/PF set, stc then sets CF
        flags = 0x45u;
    } else {
        // cmp byte flags, then clc clears CF
        const uint32_t result = (left - right) & 0xffu;
        flags = parityFlag(result)
            | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | ((result & 0x80u) != 0u ? 0x80u : 0u)
            | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004081fc, FUN_004081fc, "regs(esi:ptr[0xec]:bytes) -> (cf, pf, af, zf, sf, of)")
