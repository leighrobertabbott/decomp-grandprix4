#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// Flags of a dword-wide logic result (test eax, imm): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// Flags of a byte-wide logic result (test byte, imm): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_004110dd {
    uint8_t Unknown00[0x40];
    uint32_t State40;
    uint8_t Unknown44[0x7b - 0x44];
    uint8_t Flags7B;
    uint8_t Unknown7C[0xfe - 0x7c];
    uint8_t FlagsFE;
};

// Test on the record at esi: CF = 0 (clc) only when (+0x40 & 0x15000) == 0, (+0x40 & 0x280) != 0x280,
// +0x7b bit 1 and +0xfe bit 5 are set; otherwise CF = 1 (stc). The other flags are those of the
// last test / cmp executed. eax is saved and restored.
// FUNCTION: GP4 0x004110dd
void __cdecl FUN_004110dd(gp4::Regs* r) {
    const Unknown_004110dd* rec = reinterpret_cast<const Unknown_004110dd*>(static_cast<uintptr_t>(r->esi));
    const uint32_t state = rec->State40;

    uint32_t flags;
    bool refused;
    const uint32_t busy = state & 0x15000u;
    flags = logicFlags32(busy);
    if (busy != 0u) {
        refused = true;
    } else {
        const uint32_t both = state & 0x280u;
        flags = subFlags(both, 0x280u);
        if (both == 0x280u) {
            refused = true;
        } else {
            const uint32_t bitA = rec->Flags7B & 2u;
            flags = logicFlags8(bitA);
            if (bitA == 0u) {
                refused = true;
            } else {
                const uint32_t bitB = rec->FlagsFE & 0x20u;
                flags = logicFlags8(bitB);
                refused = (bitB == 0u);
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | (refused ? 1u : 0u);
}
GP4_IMPL(0x004110dd, FUN_004110dd, "regs(esi:ptr[0x106]:bytes) -> (cf, pf, af, zf, sf, of)")
