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

// Flags of `and eax, 0xff`: the result is a byte, so SF = 0 and CF = OF = AF = 0.
static inline uint32_t byteMaskFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u);
}

// Returns the byte at index eax of the table at esi + 0x199 when 0 <= eax < DAT_00601fb0, 0 for a
// negative index, and the word DAT_007c2e96 otherwise. ecx is saved and restored; the flags are
// those of the last cmp / and.
// FUNCTION: GP4 0x00408936
void __cdecl FUN_00408936(gp4::Regs* r) {
    const int32_t index = static_cast<int32_t>(r->eax);
    uint32_t flags = subFlags32(static_cast<uint32_t>(index), 0u);
    uint32_t result;
    if (index < 0) {
        result = 0u;
    } else {
        const int32_t limit = GP4_GLOBAL(int32_t, 0x00601fb0);
        flags = subFlags32(static_cast<uint32_t>(index), static_cast<uint32_t>(limit));
        if (index < limit) {
            const uint8_t* entry = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi + 0x199u + static_cast<uint32_t>(index)));
            result = *entry;
            flags = byteMaskFlags(result);
        } else {
            result = GP4_GLOBAL(uint16_t, 0x007c2e96);
        }
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408936, FUN_00408936, "regs(eax:i32[-4..40], esi:ptr[0x1d0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
