#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit logic result (and / or): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    return parityFlag(result) | ((result & 0xffffu) == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0040faab
// pushal ... popal routine. Clears the dword 0x007ab014 and byte 0x007a8dfa; when byte
// 0x007aafd4 is set it resets the output buffer pointers (0x004453bc) and runs 0x00445303; when
// the mode byte 0x007c2ea9 is 0x80 it runs 0x00442ed6. The word 0x007ad892 is saved to 0x0063b940
// and has bit 6 set or cleared by the byte 0x0062fb4b. Registers are restored; the flags are those
// of the final "or" / "and" on the word.
void __cdecl FUN_0040faab(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint32_t, 0x007ab014) = 0;
    GP4_GLOBAL(uint8_t, 0x007a8dfa) = 0;

    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
        gp4::call_regs(0x004453bc, c);
        gp4::call_regs(0x00445303, c);
    }
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        gp4::call_regs(0x00442ed6, c);
    }

    const uint32_t state = GP4_GLOBAL(uint16_t, 0x007ad892);
    GP4_GLOBAL(uint16_t, 0x0063b940) = static_cast<uint16_t>(state);

    uint32_t updated;
    if (GP4_GLOBAL(uint8_t, 0x0062fb4b) != 0u) {
        updated = static_cast<uint16_t>(state | 0x40u);
    } else {
        updated = static_cast<uint16_t>(state & 0xffbfu);
    }
    GP4_GLOBAL(uint16_t, 0x007ad892) = static_cast<uint16_t>(updated);

    r->eflags = (r->eflags & ~0x8d5u) | logicFlags16(updated);
}
GP4_IMPL(0x0040faab, FUN_0040faab, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+carx_0042fe2f+cur_00852d30+state_0040faab")
