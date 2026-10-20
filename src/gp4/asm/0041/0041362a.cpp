#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of SHL r32, count (count > 1) in the pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31(value << (count - 1) ^ result).
static inline uint32_t shlFlags32(uint32_t value, uint32_t count) {
    const uint32_t result = value << count;
    const uint32_t previous = value << (count - 1);
    return (previous >> 31)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Blends two values by the percentage byte [esi+0x178]: ebp = ([esi+0x178] << 14) / 100 (a Q14
// fraction), then
//   eax = (ecx * ebp >> 14) + (result of 0x00413606 * (0x4000 - ebp) >> 14)
// where ecx is the incoming eax and both products are signed 64-bit (imul, shrd 14). edx leaves holding
// the second term, ebp the fraction; the flags are those of the final add.
// FUNCTION: GP4 0x0041362a
void __cdecl FUN_0041362a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t percent = GP4_FIELD(uint8_t, record, 0x178);
    const uint32_t numerator = percent << 14;
    const uint32_t blend = numerator / 100u;
    const uint32_t remainder = numerator % 100u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ecx = r->eax;
    c.eax = blend;
    c.edx = remainder;
    c.ebp = blend;
    // idiv leaves the flags of the preceding "shl eax, 14".
    c.eflags = (r->eflags & ~0x8d5u) | shlFlags32(percent, 14u);
    gp4::call_regs(0x00413606, c);

    const uint32_t complement = 0x4000u - c.ebp;
    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(c.eax)) * static_cast<int64_t>(static_cast<int32_t>(complement));
    const uint32_t firstTerm = static_cast<uint32_t>(first >> 14);
    const int64_t second = static_cast<int64_t>(static_cast<int32_t>(c.ecx)) * static_cast<int64_t>(static_cast<int32_t>(c.ebp));
    const uint32_t secondTerm = static_cast<uint32_t>(second >> 14);

    r->eax = secondTerm + firstTerm;
    r->ecx = c.ecx;
    r->edx = firstTerm;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(secondTerm, firstTerm);
}
GP4_IMPL(0x0041362a, FUN_0041362a, "regs(eax:u32, esi:ptr[0x242]:bytes, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_00413090+engine_00413606")
