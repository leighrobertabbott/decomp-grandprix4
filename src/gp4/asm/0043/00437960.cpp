#include <gp4/gp4.h>

struct Unknown_00437960 {
    uint8_t Unknown00[0x40];
    uint32_t Unknown40;
    uint8_t Unknown44[0x80 - 0x44];
    uint8_t Flags80;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of or/test on a 32-bit result: CF=OF=AF=0
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// flags of "or byte ptr [mem], imm": CF=OF=AF=0
static inline uint32_t logicFlags8(uint8_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Translates the state bits at +0x40 into the status bits at +0x80.
// FUNCTION: GP4 0x00437960
void __cdecl FUN_00437960(gp4::Regs* r) {
    Unknown_00437960* body = reinterpret_cast<Unknown_00437960*>(r->esi);
    const uint32_t state = body->Unknown40;
    uint32_t flags = logicFlags32(state);

    if (state != 0u) {
        flags = logicFlags32(state & 0x10000u);
        if ((state & 0x10000u) != 0u) {
            body->Flags80 |= 0x20u;
            flags = logicFlags8(body->Flags80);
        }
        flags = logicFlags32(state & 0x4000u);
        if ((state & 0x4000u) != 0u) {
            body->Flags80 |= 0x10u;
            flags = logicFlags8(body->Flags80);
        }
        flags = logicFlags32(state & 0x200u);
        if ((state & 0x200u) != 0u) {
            body->Flags80 |= 0x08u;
            flags = logicFlags8(body->Flags80);
        }
        flags = logicFlags32(state & 0x80u);
        if ((state & 0x80u) != 0u) {
            body->Flags80 |= 0x04u;
            flags = logicFlags8(body->Flags80);
        }
    }

    r->eax = state;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00437960, FUN_00437960, "regs(esi:ptr[0x88]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
