#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

struct Unknown_00412b88 {
    uint8_t Unknown00[0x96];
    int16_t Unknown96;
};

// Finds the first of the two (0x00601c38[k], 0x00601c40[k]) thresholds that the record's slot
// index (0x00432c21) and its word at +0x96 fall under; edx = k, or -1 when none does. eax and ecx
// are saved and restored; the flags are those of the last compare.
// FUNCTION: GP4 0x00412b88
void __cdecl FUN_00412b88(gp4::Regs* r) {
    const Unknown_00412b88* body = reinterpret_cast<const Unknown_00412b88*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00432c21, callee);
    const int32_t index = static_cast<int32_t>(callee.eax);

    const int32_t* firstLimit = GP4_ARRAY(int32_t, 0x00601c38);
    const int32_t* secondLimit = GP4_ARRAY(int32_t, 0x00601c40);

    int32_t slot = -1;
    uint32_t flags = subFlags32(static_cast<uint32_t>(index), 0u);
    if (index >= 0) {
        slot = 0;
        for (;;) {
            flags = subFlags32(static_cast<uint32_t>(index), static_cast<uint32_t>(firstLimit[slot]));
            if (index < firstLimit[slot]) break;
            if (index <= firstLimit[slot]) {
                const int32_t remaining = static_cast<int32_t>(0x400u - static_cast<uint32_t>(static_cast<int32_t>(body->Unknown96)));
                flags = subFlags32(static_cast<uint32_t>(remaining), static_cast<uint32_t>(secondLimit[slot]));
                if (remaining < secondLimit[slot]) break;
            }
            ++slot;
            flags = subFlags32(static_cast<uint32_t>(slot), 2u);
            if (slot >= 2) {
                slot = -1;
                break;
            }
        }
    }
    r->edx = static_cast<uint32_t>(slot);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00412b88, FUN_00412b88, "regs(esi:ptr[0x105]:layout=rec_00412b88) -> (edx, cf, pf, af, zf, sf, of); globals=layout=state_00412b88")
