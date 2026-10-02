#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

struct Unknown_00434875 {
    uint8_t Unknown00[0x84];
    uint8_t Unknown84;
    uint8_t Unknown85[0x8d - 0x85];
    uint8_t Flags8D;
};

// FUNCTION: GP4 0x00434875
void __cdecl FUN_00434875(gp4::Regs* r) {
    const Unknown_00434875* body = reinterpret_cast<const Unknown_00434875*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint32_t masked = body->Flags8D & 0x10u;
    if (masked == 0u) {
        // test byte [esi+0x8d], 0x10 -> je -> clc: CF = OF = AF = 0, ZF and PF set
        flags = 0x44u;
    } else {
        // cmp byte [esi+0x84], 4; stc when equal, clc otherwise
        const uint32_t left = body->Unknown84;
        const uint32_t result = (left - 4u) & 0xffu;
        flags = parityFlag(result)
            | (((left ^ 4u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80u) != 0u ? 0x80u : 0u)
            | ((((left ^ 4u) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u)
            | (left == 4u ? 1u : 0u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434875, FUN_00434875, "regs(esi:ptr[0x95]:bytes) -> (cf, pf, af, zf, sf, of)")
