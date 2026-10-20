#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004198a1
// Stores (DAT_00603560 / DAT_00603564) >> 16 as a word at [esi+0x180] (a zero divisor counts as 1).
// ebp = the divisor, edx = the idiv remainder, eax = the shifted quotient; the flags are those of
// the final "sar eax, 16".
void __cdecl FUN_004198a1(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t divisor = GP4_GLOBAL(int32_t, 0x00603564);
    if (divisor == 0) {
        divisor = 1;
    }
    const int32_t dividend = GP4_GLOBAL(int32_t, 0x00603560);
    const int32_t quotient = dividend / divisor;
    const int32_t remainder = dividend % divisor;
    const int32_t shifted = quotient >> 16;
    *reinterpret_cast<int16_t*>(record + 0x180) = (int16_t)shifted;

    const uint32_t result = (uint32_t)shifted;
    const uint32_t flags = (((uint32_t)quotient >> 15) & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);

    r->ebp = (uint32_t)divisor;
    r->edx = (uint32_t)remainder;
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004198a1, FUN_004198a1, "regs(esi:ptr[0x188]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
