#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" on a dword
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00410967
// eax = max(3, ((al * byte DAT_007c2e96) + 0x32) / 100): a percentage of al, rounded to nearest.
// edx and ebp are preserved; the flags are those of "cmp eax, 3" (mul/div leave them alone in the
// oracle, mov does not touch them).
void __cdecl FUN_00410967(gp4::Regs* r) {
    const uint32_t scaled = (r->eax & 0xffu) * (uint32_t)GP4_GLOBAL(uint8_t, 0x007c2e96);
    uint32_t percent = (scaled + 0x32u) / 100u;
    const uint32_t flags = cmpFlags32(percent, 3u);
    if ((int32_t)percent < 3) {
        percent = 3u;
    }

    r->eax = percent;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410967, FUN_00410967, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
