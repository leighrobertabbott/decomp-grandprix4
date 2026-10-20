#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "shl value, 8" in the oracle model: CF = last bit shifted out, AF = 0, OF = bit 31 of
// (value << 7) ^ (value << 8).
static inline uint32_t shiftLeft8Flags(uint32_t value) {
    const uint32_t result = value << 8;
    return ((value >> 24) & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// pushal ... popal around: 0x004154d5 (edi/esi records), then 0x00426a66 (esi), then
// DAT_006247b4 = ((DAT_00603d48 * 4 - DAT_006037fc) << 8) / DAT_00603854 as a signed 64/32
// division. The registers are restored; the flags are those left by `shl eax, 8` (idiv keeps them).
// FUNCTION: GP4 0x00415595
void __cdecl FUN_00415595(gp4::Regs* r) {
    gp4::Regs live = *r;
    live.st_in = 0;
    live.st_out = 0;
    gp4::call_regs(0x004154d5, live);
    gp4::call_regs(0x00426a66, live);

    const uint32_t value = (0u - GP4_GLOBAL(uint32_t, 0x006037fc)) + (GP4_GLOBAL(uint32_t, 0x00603d48) << 2);
    const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(value)) * 256;
    int32_t remainder;
    GP4_GLOBAL(int32_t, 0x006247b4) = gp4::idiv64(numerator, GP4_GLOBAL(int32_t, 0x00603854), &remainder);

    r->eflags = (r->eflags & ~0x8d5u) | shiftLeft8Flags(value);
}
GP4_IMPL(0x00415595, FUN_00415595, "regs(edi:ptr[0x40]:bytes, esi:ptr[0x2a0]:layout=rec_00427880) -> (cf, pf, af, zf, sf, of); globals=layout=state_00427880")
