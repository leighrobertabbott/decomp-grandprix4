#include <gp4/gp4.h>

// Flags left by "shl reg32, 8" under the pinned oracle model:
// CF = last bit shifted out, AF = 0, OF = bit31((value << 7) ^ result).
static inline uint32_t Shl8Flags(uint32_t value) {
    const uint32_t before = value << 7;
    const uint32_t result = value << 8;
    return ((before >> 31) & 1u)
        | 0x4u  // low byte of the result is zero: even parity
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((before ^ result) >> 31) << 11);
}

// FUNCTION: GP4 0x004120e4
void __cdecl FUN_004120e4(gp4::Regs* r) {
    uint8_t* state = GP4_ARRAY(uint8_t, 0x006380d8);
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x00601334);
    int32_t rem;

    // Two scaled quotients ((x << 14) / divisor) >> 1, each copied to two slots.
    const uint32_t x0 = GP4_GLOBAL(uint32_t, 0x00601350);
    const int32_t q0 = gp4::idiv64(static_cast<int64_t>(static_cast<uint64_t>(x0) << 14), divisor, &rem) >> 1;
    GP4_FIELD(int32_t, state, 0x198) = q0;
    GP4_FIELD(int32_t, state, 0x498) = q0;
    const uint32_t x1 = GP4_GLOBAL(uint32_t, 0x0060134c);
    const int32_t q1 = gp4::idiv64(static_cast<int64_t>(static_cast<uint64_t>(x1) << 14), divisor, &rem) >> 1;
    GP4_FIELD(int32_t, state, 0x798) = q1;
    GP4_FIELD(int32_t, state, 0xa98) = q1;

    // Common factor: (DAT_0062ee00 * DAT_00602a48) >> 28 plus DAT_00601388.
    const int64_t factorProduct = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062ee00))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00602a48));
    const uint32_t factor = static_cast<uint32_t>(static_cast<uint64_t>(factorProduct) >> 28)
        + GP4_GLOBAL(uint32_t, 0x00601388);

    const int64_t p0 = static_cast<int64_t>(GP4_FIELD(int32_t, state, 0x198)) * static_cast<int32_t>(factor);
    const uint32_t s0 = static_cast<uint32_t>(static_cast<uint64_t>(p0) >> 14);
    const uint32_t v0 = s0 << 8;
    GP4_FIELD(uint32_t, state, 0x94) = v0;
    GP4_GLOBAL(uint32_t, 0x00601d7c) = v0;
    GP4_FIELD(uint32_t, state, 0xb8) = v0;
    GP4_FIELD(uint32_t, state, 0x394) = v0;
    GP4_GLOBAL(uint32_t, 0x00601d80) = v0;
    GP4_FIELD(uint32_t, state, 0x3b8) = v0;

    const int64_t p1 = static_cast<int64_t>(GP4_FIELD(int32_t, state, 0x798)) * static_cast<int32_t>(factor);
    const uint32_t s1 = static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14);
    const uint32_t v1 = s1 << 8;
    GP4_FIELD(uint32_t, state, 0x694) = v1;
    GP4_GLOBAL(uint32_t, 0x00601d84) = v1;
    GP4_FIELD(uint32_t, state, 0x6b8) = v1;
    GP4_FIELD(uint32_t, state, 0x994) = v1;
    GP4_GLOBAL(uint32_t, 0x00601d88) = v1;
    GP4_FIELD(uint32_t, state, 0x9b8) = v1;

    const uint32_t a = GP4_GLOBAL(uint32_t, 0x00601394);
    GP4_FIELD(uint32_t, state, 0x9c) = a;
    GP4_FIELD(uint32_t, state, 0x39c) = a;
    const uint32_t b = GP4_GLOBAL(uint32_t, 0x00601390);
    GP4_FIELD(uint32_t, state, 0x69c) = b;
    GP4_FIELD(uint32_t, state, 0x99c) = b;

    r->eax = b;
    r->ebx = 0x006380d8u;
    r->ecx = factor;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | Shl8Flags(s1);
}
GP4_IMPL(0x004120e4, FUN_004120e4, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_004120e4")
