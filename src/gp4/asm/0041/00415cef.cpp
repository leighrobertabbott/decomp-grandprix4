#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (32-bit) without CF.
static inline uint32_t compareFlagsNoCarry(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Compares the word at +0x62 with min(word at +0x264 + DAT_00603aac, DAT_00601314) (signed): CF = 1
// when the first is not below the second (stc), else 0 (clc); the other flags are those of that
// "cmp eax, edx". eax and edx are saved and restored.
// FUNCTION: GP4 0x00415cef
void __cdecl FUN_00415cef(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t current = static_cast<int32_t>(*reinterpret_cast<const uint16_t*>(record + 0x62));
    int32_t limit = static_cast<int32_t>(static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(record + 0x264))
        + GP4_GLOBAL(uint32_t, 0x00603aac));
    if (limit > GP4_GLOBAL(int32_t, 0x00601314)) {
        limit = GP4_GLOBAL(int32_t, 0x00601314);
    }

    const uint32_t flags = compareFlagsNoCarry(static_cast<uint32_t>(current), static_cast<uint32_t>(limit))
        | (current >= limit ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415cef, FUN_00415cef, "regs(esi:ptr[0x26c]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
