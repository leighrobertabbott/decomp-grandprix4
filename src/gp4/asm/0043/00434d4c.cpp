#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags left by a byte-wide AND/TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t LogicFlags8(uint32_t result) {
    uint32_t flags = ParityFlag(result);
    if ((result & 0xffu) == 0u) flags |= 0x40u;
    if ((result & 0x80u) != 0u) flags |= 0x80u;
    return flags;
}

// FUNCTION: GP4 0x00434d4c
void __cdecl FUN_00434d4c(gp4::Regs* r) {
    void* record = (void*)r->esi;

    // test byte [esi+0x161], 2
    const uint8_t gate = GP4_FIELD(uint8_t, record, 0x161);
    uint32_t flags = LogicFlags8(gate & 2u);
    if ((gate & 2u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x161) = (uint8_t)(gate & 0xfdu);
        GP4_FIELD(uint8_t, record, 0x162) &= 0x7f;
        const uint8_t status = (uint8_t)(GP4_FIELD(uint8_t, record, 0x163) & 0x7fu);
        GP4_FIELD(uint8_t, record, 0x163) = status;
        flags = LogicFlags8(status);   // the final `and` leaves the flags
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434d4c, FUN_00434d4c, "regs(esi:ptr[0x16b]:bytes) -> (cf, pf, af, zf, sf, of)")
