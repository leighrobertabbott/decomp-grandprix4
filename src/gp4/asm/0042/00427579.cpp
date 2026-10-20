#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left + right);
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of a 16-bit `sub left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00427579_Mode {
    uint8_t Unknown00[0x71];
    uint8_t Unknown71;
};

struct Unknown_00427579_Pair {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x1c0 - 0x64];
    uint16_t Unknown1C0;
    uint16_t Unknown1C2;
};

// Adds to ax a slope term: 8 * (difference of two word pairs of edi, picked by bit 3 of the
// byte at esi+0x71), clamped to +-0x1000. When the difference is zero eax is unchanged (the flags
// then are those of the zero "sub"). The high half of eax holds the clamped term's high half.
// edx is preserved.
// FUNCTION: GP4 0x00427579
void __cdecl FUN_00427579(gp4::Regs* r) {
    const Unknown_00427579_Mode* mode = reinterpret_cast<const Unknown_00427579_Mode*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00427579_Pair* pair = reinterpret_cast<const Unknown_00427579_Pair*>(static_cast<uintptr_t>(r->edi));
    const uint32_t savedEax = r->eax;

    uint16_t left;
    uint16_t right;
    if ((mode->Unknown71 & 8u) == 0u) {
        left = pair->Unknown1C2;
        right = pair->Unknown62;
    } else {
        left = pair->Unknown60;
        right = pair->Unknown1C0;
    }
    const uint16_t difference = static_cast<uint16_t>(left - right);

    if (difference == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | subFlags16(left, right);
        return;
    }

    int32_t term = static_cast<int32_t>(static_cast<int16_t>(difference)) << 3;
    if (term < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(term);
        if (magnitude >= 0x1000u) {
            magnitude = 0x1000u;
        }
        term = static_cast<int32_t>(0u - magnitude);
    } else if (static_cast<uint32_t>(term) >= 0x1000u) {
        term = 0x1000;
    }

    const uint16_t low = static_cast<uint16_t>(term);
    const uint16_t addend = static_cast<uint16_t>(savedEax);
    r->eax = (static_cast<uint32_t>(term) & 0xffff0000u) | static_cast<uint16_t>(low + addend);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(low, addend);
}
GP4_IMPL(0x00427579, FUN_00427579, "regs(eax:u32, esi:ptr[0x79]:bytes, edi:ptr[0x1cc]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
