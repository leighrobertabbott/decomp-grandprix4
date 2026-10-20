#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// When the index DAT_0062789c is below 4 (signed), sets its bit in the bit string at 0x00601eb0 if
// DAT_0062783c >= DAT_00891500 (unsigned) and clears it otherwise (bts / btr with a register bit
// offset, which addresses dword index >> 5). eax and edx are saved and restored; the flags are those
// of the last cmp with CF replaced by the bit's previous value.
// FUNCTION: GP4 0x0041dbb0
void __cdecl FUN_0041dbb0(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x0062789c);
    uint32_t flags = subFlags32(index, 4u);
    if (static_cast<int32_t>(index) < 4) {
        const uint32_t current = GP4_GLOBAL(uint32_t, 0x0062783c);
        const uint32_t reference = GP4_GLOBAL(uint32_t, 0x00891500);
        flags = subFlags32(current, reference);

        uint32_t* word = GP4_ARRAY(uint32_t, 0x00601eb0) + (static_cast<int32_t>(index) >> 5);
        const uint32_t mask = 1u << (index & 31u);
        const uint32_t previous = (*word & mask) != 0u ? 1u : 0u;
        if (current >= reference) {
            *word |= mask;
        } else {
            *word &= ~mask;
        }
        flags = (flags & ~1u) | previous;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041dbb0, FUN_0041dbb0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0041dbb0")
