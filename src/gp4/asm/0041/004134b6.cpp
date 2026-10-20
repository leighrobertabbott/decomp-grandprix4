#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004134b6
// DAT_006036dc = sign-extended(min-clamped |word [esi+0xe] - word [edi]|) ^ DAT_00638ce0, where a
// magnitude above 0x4000 becomes 0x8000. eax holds the result and the flags are those of the final
// "xor" (CF = OF = AF = 0).
void __cdecl FUN_004134b6(gp4::Regs* r) {
    uint8_t* from = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* to = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    int16_t difference = (int16_t)(GP4_FIELD(uint16_t, from, 0xe) - GP4_FIELD(uint16_t, to, 0));
    if (difference < 0) {
        difference = (int16_t)(0 - difference);
    }
    if (difference > 0x4000) {
        difference = (int16_t)0x8000;
    }

    const uint32_t result = (uint32_t)(int32_t)difference ^ GP4_GLOBAL(uint32_t, 0x00638ce0);
    GP4_GLOBAL(uint32_t, 0x006036dc) = result;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}
GP4_IMPL(0x004134b6, FUN_004134b6, "regs(esi:ptr[0x18]:bytes, edi:ptr[0x8]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
