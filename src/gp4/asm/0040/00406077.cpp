#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "sub left, right" (32-bit).
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Steps the record pointer in edi back one 0x160-byte record, wrapping at the array ends: below
// DAT_00891500 the array base 0x008943a0 wraps to DAT_00866ecc, and the pointer equal to
// DAT_00891500 wraps to DAT_00866f00. The flags are those of the final "sub edi, 0x160".
// FUNCTION: GP4 0x00406077
void __cdecl FUN_00406077(gp4::Regs* r) {
    uint32_t record = r->edi;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (record < limit) {
        if (record == 0x008943a0u) {
            record = GP4_GLOBAL(uint32_t, 0x00866ecc);
        }
    } else if (record == GP4_GLOBAL(uint32_t, 0x00891500)) {
        record = GP4_GLOBAL(uint32_t, 0x00866f00);
    }

    const uint32_t flags = subtractFlags32(record, 0x160u);
    r->edi = record - 0x160u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00406077, FUN_00406077, "regs(edi:u32[0x8943a0..0x8943a3]) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
