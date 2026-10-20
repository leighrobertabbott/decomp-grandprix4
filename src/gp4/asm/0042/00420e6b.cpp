#include <gp4/gp4.h>

struct Unknown_00420e6b {
    uint8_t Unknown00[0x32c];
    int32_t Unknown32C[4];     // per-car clamp, lowered to the record's +0x1c8 value
    uint8_t Unknown33C[0x34c - 0x33c];
    int32_t Unknown34C[4];     // cleared whenever the matching +0x32c value changes
};

struct Unknown_006380d8 {
    uint8_t Unknown00[0x1c8];
    int32_t Unknown1C8;
    uint8_t Unknown1CC[0x300 - 0x1cc];
};

// For each of the four 0x300-byte records at 0x6380d8 (index k): the signed value +0x32c[k] of
// the block at esi is lowered to the record's +0x1c8; a lowered or negative value also clears
// +0x34c[k] (a negative value becomes 0). pushal/popal keep every register; the flags are those of
// the final "cmp ebx, 0x638cd8" (equal: ZF and PF).
// FUNCTION: GP4 0x00420e6b
void __cdecl FUN_00420e6b(gp4::Regs* r) {
    Unknown_00420e6b* block = reinterpret_cast<Unknown_00420e6b*>(static_cast<uintptr_t>(r->esi));
    const Unknown_006380d8* record = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    uint32_t index = 0;
    do {
        const int32_t current = block->Unknown32C[index];
        if (current > record->Unknown1C8) {
            block->Unknown32C[index] = record->Unknown1C8;
            block->Unknown34C[index] = 0;
        } else if (block->Unknown32C[index] < 0) {
            block->Unknown32C[index] = 0;
            block->Unknown34C[index] = 0;
        }
        ++index;
        ++record;
    } while (reinterpret_cast<uintptr_t>(record) < 0x00638cd8u);

    // popal leaves the flags of the last "cmp ebx, 0x638cd8" (equal)
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00420e6b, FUN_00420e6b, "regs(esi:ptr[0x354]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=lim_00420e6b")
