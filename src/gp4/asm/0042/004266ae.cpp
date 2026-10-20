#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `left - right` (cmp).
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// DAT_006247c4 = 0x10 when the dword at +0xbc is zero, else min(word[+0x100], 0x1d55).
// All registers are saved (pushal/popal) but popal does not restore the flags, so they are those
// of the last flag writer: the dword test (value 0) or the cmp against 0x1d55.
// FUNCTION: GP4 0x004266ae
void __cdecl FUN_004266ae(gp4::Regs* r) {
    uint8_t* owner = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t value;
    uint32_t flags;

    if (GP4_FIELD(uint32_t, owner, 0xbc) == 0u) {
        value = 0x10u;
        flags = 0x44u;                                        // test of zero: ZF | PF
    } else {
        value = GP4_FIELD(uint16_t, owner, 0x100);           // movzx eax, word
        flags = subtractionFlags32(value, 0x1d55u);          // cmp eax, 0x1d55
        if (static_cast<int32_t>(value) > 0x1d55) {
            value = 0x1d55u;
        }
    }
    GP4_GLOBAL(uint32_t, 0x006247c4) = value;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004266ae, FUN_004266ae, "regs(esi:ptr[0x110]:layout=rec_004266ae) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
