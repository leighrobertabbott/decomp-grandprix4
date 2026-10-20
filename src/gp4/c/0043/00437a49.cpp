#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 8-bit `left - right` (both operands already in 0..255).
static inline uint32_t subFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) << 4);
}

// Flags of a byte-wide logic result (or / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_00437a49 {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0xfe - 0x72];
    uint8_t FlagsFE;
    uint8_t Unknown00FF[0x230 - 0xff];
    uint8_t Flags230;
};

// Sets the status bits of the record at esi (+0xfe bit 2, +0x71 bit 7, clears +0x230 bit 7)
// unless byte 0x007c2ea9 equals 0x80 and (byte 0x0066d8c8 + 1) >= byte 0x007c2e96 (signed).
// al is clobbered by the second comparison; the flags are those of the last flag writer.
// FUNCTION: GP4 0x00437a49
void __cdecl FUN_00437a49(gp4::Regs* r) {
    Unknown_00437a49* rec = reinterpret_cast<Unknown_00437a49*>(r->esi);
    uint32_t eax = r->eax;

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = subFlags8(mode, 0x80u);
    bool update = (mode != 0x80u);
    if (!update) {
        const uint32_t next = (static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x0066d8c8)) + 1u) & 0xffu;
        eax = (eax & 0xffffff00u) | next;
        const uint32_t limit = GP4_GLOBAL(uint8_t, 0x007c2e96);
        flags = subFlags8(next, limit);
        update = static_cast<int8_t>(next) < static_cast<int8_t>(limit);
    }
    if (update) {
        rec->FlagsFE = static_cast<uint8_t>(rec->FlagsFE | 0x04u);
        rec->Flags230 = static_cast<uint8_t>(rec->Flags230 & 0x7fu);
        rec->Flags71 = static_cast<uint8_t>(rec->Flags71 | 0x80u);
        flags = logicFlags8(rec->Flags71);
    }

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00437a49, FUN_00437a49, "regs(eax:u32, esi:ptr[0x238]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
