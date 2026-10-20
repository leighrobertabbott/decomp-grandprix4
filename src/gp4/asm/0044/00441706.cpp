#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `shr value, 16` under the oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31((value >> 15) ^ result) = 0, ZF/SF/PF from the result.
static inline uint32_t shr16Flags32(uint32_t value) {
    const uint32_t result = value >> 16;
    return ((value >> 15) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((((value >> 15) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Record {
    uint8_t Unknown00[0x7c];
    uint8_t Index7C;
};

// Copies the high words of four per-slot tables (0x0066fb80 / 0x0066fbd8 / 0x0066fc30 / 0x0066fc88, one dword
// per slot) for slot (byte +0x7c of the record in esi) - 1, wrapped to 0..63, into 0x007a9104..0x007a9110.
// eax ends as the last value, edx as the slot index; the flags are those of the last shr.
// FUNCTION: GP4 0x00441706
void __cdecl FUN_00441706(gp4::Regs* r) {
    const Unknown_Record* record = reinterpret_cast<const Unknown_Record*>(static_cast<uintptr_t>(r->esi));
    const uint32_t slot = static_cast<uint8_t>(record->Index7C - 1u) & 0x3fu;

    GP4_GLOBAL(uint32_t, 0x007a9104) = GP4_ARRAY(uint32_t, 0x0066fb80)[slot] >> 16;
    GP4_GLOBAL(uint32_t, 0x007a9108) = GP4_ARRAY(uint32_t, 0x0066fbd8)[slot] >> 16;
    GP4_GLOBAL(uint32_t, 0x007a910c) = GP4_ARRAY(uint32_t, 0x0066fc30)[slot] >> 16;
    const uint32_t last = GP4_ARRAY(uint32_t, 0x0066fc88)[slot];
    GP4_GLOBAL(uint32_t, 0x007a9110) = last >> 16;

    r->eax = last >> 16;
    r->edx = slot;
    r->eflags = (r->eflags & ~0x8d5u) | shr16Flags32(last);
}
GP4_IMPL(0x00441706, FUN_00441706, "regs(esi:ptr[0x84]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=slots_0066fb80")
