#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of SHRD r32, r32, count (count > 1) in the pinned oracle model.
static inline uint32_t shrdFlags32(uint32_t low, uint32_t high, uint32_t count) {
    const uint64_t joined = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t result = static_cast<uint32_t>(joined >> count);
    const uint32_t previous = static_cast<uint32_t>(joined >> (count - 1));
    return (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_004132e5_Body {
    uint8_t Unknown00[0x162];
    uint8_t Unknown162;
    uint8_t Unknown163[0x1b6 - 0x163];
    uint16_t Unknown1B6;
    uint8_t Unknown1B8[0x2ec - 0x1b8];
    uint32_t Unknown2EC;
    uint32_t Unknown2F0;
    uint32_t Unknown2F4;
    uint32_t Unknown2F8;
};

// FUNCTION: GP4 0x004132e5
// esi = body. Computes a Q14 factor and multiplies the global 0x006036a8 by it (>> 14).
// Bit 3 of +0x162 set: factor = the 0x00601c50 table entry selected by 0x00412b88 (edx = index),
// or the value of 0x00601c14 when it returns -1. Otherwise: ratio = ((+0x2f4 + +0x2f8) << 14) /
// max-nonzero((+0x2ec + .. + +0x2f8) * word(+0x1b6) >> 14) is stored to 0x006030d8, clamped to
// 0..0x4000, and the factor is 0x4000 - ((0x4000 - ratio) * [0x006030e8] >> 14).
// eax = new 0x006036a8, edx = high half of the final product, ebp = the divisor (second path only),
// flags of the final shrd.
void __cdecl FUN_004132e5(gp4::Regs* r) {
    const Unknown_004132e5_Body* body = reinterpret_cast<const Unknown_004132e5_Body*>(static_cast<uintptr_t>(r->esi));
    uint32_t ebp = r->ebp;
    uint32_t factor;

    if ((body->Unknown162 & 8u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = GP4_GLOBAL(uint32_t, 0x00601c14);
        gp4::call_regs(0x00412b88, call);
        factor = call.eax;
        if (call.edx != 0xffffffffu) {
            factor = GP4_ARRAY(uint32_t, 0x00601c50)[call.edx];
        }
        ebp = call.ebp;
    } else {
        const uint32_t numerator = body->Unknown2F4 + body->Unknown2F8;
        const uint32_t total = body->Unknown2EC + body->Unknown2F0 + numerator;
        const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(total)) * static_cast<int64_t>(body->Unknown1B6);
        ebp = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
        if (ebp == 0u) {
            ebp = 1u;
        }
        const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(numerator))) << 14);
        int32_t remainder;
        int32_t ratio = gp4::idiv64(dividend, static_cast<int32_t>(ebp), &remainder);
        GP4_GLOBAL(int32_t, 0x006030d8) = ratio;
        if (ratio > 0x4000) {
            ratio = 0x4000;
        }
        if (ratio < 0) {
            ratio = 0;
        }
        const uint32_t remaining = 0x4000u - static_cast<uint32_t>(ratio);
        const int64_t weighted = static_cast<int64_t>(static_cast<int32_t>(remaining)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006030e8));
        const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(weighted) >> 14);
        factor = 0x4000u - shifted;
    }

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(factor)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006036a8));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    GP4_GLOBAL(uint32_t, 0x006036a8) = result;

    r->eax = result;
    r->edx = high;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | shrdFlags32(low, high, 14);
}
GP4_IMPL(0x004132e5, FUN_004132e5, "regs(esi:ptr[0x300]:layout=rec_004132e5+rec_0041325b+rec_00412b88) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00412b88")
