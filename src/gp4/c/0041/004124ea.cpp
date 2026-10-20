#include <gp4/gp4.h>

// Flags left by "shl reg32, 20" under the pinned oracle model (the following DIV keeps them):
// CF = last bit shifted out, AF = 0, OF = bit31((value << 19) ^ result).
static inline uint32_t Shl20Flags(uint32_t value) {
    const uint32_t before = value << 19;
    const uint32_t result = value << 20;
    return ((before >> 31) & 1u)
        | 0x4u  // low byte of the result is zero: even parity
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((before ^ result) >> 31) << 11);
}

// FUNCTION: GP4 0x004124ea
void __cdecl FUN_004124ea(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00603854);
    const int32_t value16 = static_cast<int16_t>(value);
    uint32_t rem;

    // (int16)value * 0x4ce0 >> 14, kept as 16 bits.
    const int32_t scaled = value16 * 0x4ce0;
    GP4_GLOBAL(uint16_t, 0x00623df4) = static_cast<uint16_t>(static_cast<uint32_t>(scaled) >> 14);

    // 0x4000000 / value, and (0x517d * value) >> 14 as 16 bits.
    GP4_GLOBAL(uint32_t, 0x00623df6) = gp4::div64(0x4000000ull, value, &rem);
    const uint64_t wide = static_cast<uint64_t>(0x517du) * value;
    GP4_GLOBAL(uint16_t, 0x00623e06) = static_cast<uint16_t>(static_cast<uint32_t>(wide >> 14));

    // Nine... eighteen words scaled by value16 (high half of the 16x16 product).
    const int16_t* src = GP4_ARRAY(int16_t, 0x00603978);
    int16_t* dst = GP4_ARRAY(int16_t, 0x0060399c);
    for (int i = 0; i < 18; ++i) {
        dst[i] = static_cast<int16_t>((static_cast<int32_t>(src[i]) * value16) >> 16);
    }
    GP4_GLOBAL(int16_t, 0x00623e02) = static_cast<int16_t>(
        (static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint32_t, 0x006037ec))) * value16) >> 16);
    GP4_GLOBAL(int16_t, 0x00623e04) = static_cast<int16_t>(
        (static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint32_t, 0x006037f0))) * value16) >> 16);

    // 0x4000 - (max(0x2000 - value, 0) * DAT_006039c0 >> 14), capped at 0x4000.
    int32_t headroom = static_cast<int32_t>(0x2000u - value);
    if (headroom < 0) headroom = 0;
    const int64_t product = static_cast<int64_t>(headroom) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006039c0));
    int32_t blend = static_cast<int32_t>(0x4000u - static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    if (blend > 0x4000) blend = 0x4000;
    GP4_GLOBAL(int32_t, 0x006039c4) = blend;

    // (value << 20) / DAT_006013d8; the DIV leaves the flags of the SHL.
    const uint32_t quotient = gp4::div64(static_cast<uint64_t>(value) << 20, GP4_GLOBAL(uint32_t, 0x006013d8), &rem);
    GP4_GLOBAL(uint32_t, 0x00601cc8) = quotient;

    r->eax = quotient;
    r->ebx = 0x0060399cu;
    r->ecx = 0x006039c0u;
    r->edx = rem;
    r->eflags = (r->eflags & ~0x8d5u) | Shl20Flags(value);
}
GP4_IMPL(0x004124ea, FUN_004124ea, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_004124ea")
