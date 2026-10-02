#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004081e6
void __cdecl FUN_004081e6(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    if ((GP4_FIELD(uint8_t, record, 0x25f) & 8u) != 0u) {
        // test result 8 (PF=0, ZF=0, SF=0), then clc
        flags = 0u;
    } else {
        const uint32_t left = GP4_FIELD(uint8_t, record, 0xe4);
        const uint32_t right = 3u;
        const uint32_t result = (left - right) & 0xffu;
        if (left == right) {
            // cmp equal: ZF|PF, then stc sets CF
            flags = 0x45u;
        } else {
            // cmp flags (CF cleared by clc)
            flags = parityFlag(result)
                | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
                | ((result & 0x80u) != 0u ? 0x80u : 0u)
                | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004081e6, FUN_004081e6, "regs(esi:ptr[0x267]:bytes) -> (cf, pf, af, zf, sf, of)")
