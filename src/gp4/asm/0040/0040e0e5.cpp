#include <gp4/gp4.h>

// (value * factor) >> 14 as `imul; shrd eax, edx, 14` leaves it: the low 32 bits of the signed 64-bit
// product shifted right by 14.
static inline int32_t mulQ14(int32_t value, int32_t factor) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(factor);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
}

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The 16-bit Q14 sine routine 0x0040119c: ax = angle in, ax = sin(angle) out.
static inline int32_t sineQ14(const gp4::Regs& base, uint16_t angle) {
    gp4::Regs rg = base;
    rg.eax = angle;
    gp4::call_regs(0x0040119c, rg);
    return static_cast<int16_t>(rg.eax & 0xffffu);
}

// Euler-angle composition. ax, bx, cx are 16-bit binary angles (halved with `sar 1`); for each the
// sine (0x0040119c at the angle) and the cosine (0x0040119c at 0x4000 - angle) are stored as sign
// extended dwords at 0x00625a74/78 (angle A), 7c/80 (B) and 84/88 (C). The Q14 products
//   eax = cosC*cosA*cosB + sinC*sinA*sinB   ebx = sinC*sinA*cosB + cosC*cosA*sinB
//   ecx = cosC*sinA*cosB - sinC*cosA*sinB   edx = cosC*sinA*sinB - sinC*cosA*cosB
// are returned in the registers (each shrd 14 step truncates); the flags are those of the last add.
// FUNCTION: GP4 0x0040e0e5
void __cdecl FUN_0040e0e5(gp4::Regs* r) {
    const uint16_t halfA = static_cast<uint16_t>(static_cast<int16_t>(r->eax & 0xffffu) >> 1);
    const uint16_t halfB = static_cast<uint16_t>(static_cast<int16_t>(r->ebx & 0xffffu) >> 1);
    const uint16_t halfC = static_cast<uint16_t>(static_cast<int16_t>(r->ecx & 0xffffu) >> 1);

    const int32_t sinA = sineQ14(*r, halfA);
    GP4_GLOBAL(int32_t, 0x00625a74) = sinA;
    const int32_t cosA = sineQ14(*r, static_cast<uint16_t>(0x4000u - halfA));
    GP4_GLOBAL(int32_t, 0x00625a78) = cosA;
    const int32_t sinB = sineQ14(*r, halfB);
    GP4_GLOBAL(int32_t, 0x00625a7c) = sinB;
    const int32_t cosB = sineQ14(*r, static_cast<uint16_t>(0x4000u - halfB));
    GP4_GLOBAL(int32_t, 0x00625a80) = cosB;
    const int32_t sinC = sineQ14(*r, halfC);
    GP4_GLOBAL(int32_t, 0x00625a84) = sinC;
    const int32_t cosC = sineQ14(*r, static_cast<uint16_t>(0x4000u - halfC));
    GP4_GLOBAL(int32_t, 0x00625a88) = cosC;

    const int32_t sinAcosB = mulQ14(sinA, cosB);
    const int32_t cosAsinB = mulQ14(cosA, sinB);
    const int32_t first = mulQ14(sinC, sinAcosB) + mulQ14(cosC, cosAsinB);
    const int32_t second = mulQ14(cosC, sinAcosB) - mulQ14(sinC, cosAsinB);

    const int32_t sinAsinB = mulQ14(sinA, sinB);
    const int32_t cosAcosB = mulQ14(cosA, cosB);
    const int32_t third = mulQ14(cosC, sinAsinB) - mulQ14(sinC, cosAcosB);
    const uint32_t lastLeft = static_cast<uint32_t>(mulQ14(cosC, cosAcosB));
    const uint32_t lastRight = static_cast<uint32_t>(mulQ14(sinC, sinAsinB));

    r->eax = lastLeft + lastRight;
    r->ebx = static_cast<uint32_t>(first);
    r->ecx = static_cast<uint32_t>(second);
    r->edx = static_cast<uint32_t>(third);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(lastLeft, lastRight);
}
GP4_IMPL(0x0040e0e5, FUN_0040e0e5, "regs(eax:u32, ebx:u32, ecx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")
