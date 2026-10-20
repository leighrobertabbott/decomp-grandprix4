#include <gp4/gp4.h>

struct UnknownBody_00425fdc {
    uint8_t Unknown00[0x8d];
    uint8_t Unknown8D;
    uint8_t Unknown8E[0xa];
    int32_t Unknown98;
    uint8_t Unknown9C[0x3a];
    uint16_t UnknownD6;
    uint8_t UnknownD8[0x172];
    uint16_t Unknown24A;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Chain of fixed-point products: table[|+0x98 >> 2| >> 20 (+0x168 when bit 0 of +0x8d)] * word(+0xd6)
// >> 14, * DAT_00603854 >> 2, * word(+0x24a) >> 14. ebx and ebp are preserved; edx leaves holding
// the high half of the last imul and the flags are those of the final shrd.
// FUNCTION: GP4 0x00425fdc
void __cdecl FUN_00425fdc(gp4::Regs* r) {
    const UnknownBody_00425fdc* body = (const UnknownBody_00425fdc*)r->esi;

    int32_t shifted = body->Unknown98 >> 2;
    if (shifted < 0) shifted = static_cast<int32_t>(0u - static_cast<uint32_t>(shifted));
    uint32_t index = static_cast<uint32_t>(shifted) >> 20;
    if ((body->Unknown8D & 1u) != 0u) index += 0x168u;

    const int64_t first = static_cast<int64_t>(GP4_ARRAY(int32_t, 0x0063a194)[index])
        * static_cast<int64_t>(body->UnknownD6);
    const int32_t step1 = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14));

    const int64_t second = static_cast<int64_t>(step1) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    const int32_t step2 = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 2));

    const int64_t third = static_cast<int64_t>(step2) * static_cast<int64_t>(body->Unknown24A);
    const uint32_t low = static_cast<uint32_t>(third);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(third) >> 32);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(third) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(third) >> 13);

    // shrd by 14: CF is the last bit shifted out; OF/AF follow the pinned oracle model
    // (OF = bit31(previous ^ result), AF = 0)
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00425fdc, FUN_00425fdc, "regs(esi:ptr[0x252]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
