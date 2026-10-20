#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Clears five bytes of the record in esi (0x6a word, 0x6c..0x6f bytes); esi may be null.
// FUNCTION: GP4 0x0040f9e6
void __cdecl FUN_0040f9e6(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = r->esi;
    // or esi, esi: CF=OF=AF=0, ZF/SF/PF from esi (the stores leave the flags alone)
    const uint32_t flags = parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
    if (value != 0u) {
        GP4_FIELD(uint8_t, record, 0x6f) = 0u;
        GP4_FIELD(uint16_t, record, 0x6a) = 0u;
        GP4_FIELD(uint8_t, record, 0x6e) = 0u;
        GP4_FIELD(uint8_t, record, 0x6c) = 0u;
        GP4_FIELD(uint8_t, record, 0x6d) = 0u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f9e6, FUN_0040f9e6, "regs(esi:nptr[0x77]:bytes) -> (cf, pf, af, zf, sf, of)")
