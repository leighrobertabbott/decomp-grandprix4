#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Record fields used here.
struct Unknown_00436eee {
    uint8_t Unknown00[0x128];
    uint32_t Unknown128;
    uint8_t Unknown12C[8];
    uint8_t Flags134;
    uint8_t Unknown135[0xb7];
    uint32_t Unknown1EC;
    uint8_t Unknown1F0[0x45];
    int8_t Counter235;
};

// FUNCTION: GP4 0x00436eee
// Marks the record at esi (flag 0x20 at +0x134) and refreshes its timer: +0x1ec = [0x0066d814] + 3000.
// When flag 0x10 is already set and +0x128 equals ebx, the counter at +0x235 is incremented and
// saturated at 100 (signed); otherwise flag 0x10 is set, +0x128 = ebx and the counter restarts at 1.
// The flags are those of the final "add [esi+0x1ec], 0xbb8".
void __cdecl FUN_00436eee(gp4::Regs* r) {
    Unknown_00436eee* record = reinterpret_cast<Unknown_00436eee*>(static_cast<uintptr_t>(r->esi));
    const uint32_t key = r->ebx;

    record->Flags134 |= 0x20u;
    if ((record->Flags134 & 0x10u) != 0u && key == record->Unknown128) {
        record->Counter235 = static_cast<int8_t>(record->Counter235 + 1);
        if (record->Counter235 > 0x64) {
            record->Counter235 = 0x64;
        }
    } else {
        record->Flags134 |= 0x10u;
        record->Unknown128 = key;
        record->Counter235 = 1;
    }

    const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
    record->Unknown1EC = now + 0xbb8u;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(now, 0xbb8u);
}
GP4_IMPL(0x00436eee, FUN_00436eee, "regs(ebx:u32[0..3], esi:ptr[0x240]:layout=rec_00436eee) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
