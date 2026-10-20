#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// 0x0040119c: ax = table lookup of the 16-bit angle in ax (sign-extended result).
static inline int32_t lookup(const gp4::Regs* r, uint32_t angle) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = angle & 0xffffu;
    gp4::call_regs(0x0040119c, c);
    return static_cast<int16_t>(c.eax);
}

// imul + shrd eax, edx, 14: (a * b) >> 14, low 32 bits.
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// Normalises the vector (eax, ebx, ecx) = (a, b, c): norm = isqrt(a^2 + b^2 + c^2) (0x0040113f,
// a zero norm is replaced by 1 as the divisor). With n = norm >> 9, the table values at
// 0x4000 - n and n (0x0040119c) give q = (cosish << 14) / norm, and the outputs are
// ebx = a*q >> 14, ecx = b*q >> 14, edx = c*q >> 14, eax = the table value at n, ebp = q. esi and
// edi are saved and restored; the flags are those of the last "shrd eax, edx, 14" (pinned oracle
// model: AF = 0, CF = bit 13 of the product, OF = bit31(product>>13 ^ product>>14)).
// FUNCTION: GP4 0x0040e46e
void __cdecl FUN_0040e46e(gp4::Regs* r) {
    const int32_t a = static_cast<int32_t>(r->eax);
    const int32_t b = static_cast<int32_t>(r->ebx);
    const int32_t c = static_cast<int32_t>(r->ecx);

    const uint64_t sumOfSquares = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(a))
        + static_cast<uint64_t>(static_cast<int64_t>(b) * static_cast<int64_t>(b))
        + static_cast<uint64_t>(static_cast<int64_t>(c) * static_cast<int64_t>(c));

    gp4::Regs root = *r;
    root.st_in = 0;
    root.st_out = 0;
    root.eax = static_cast<uint32_t>(sumOfSquares);
    root.edx = static_cast<uint32_t>(sumOfSquares >> 32);
    gp4::call_regs(0x0040113f, root);

    uint32_t divisor = root.eax;
    if (divisor == 0u) {
        divisor = 1u;
    }
    const uint32_t angle = root.eax >> 9;
    const int32_t complement = lookup(r, (0x4000u - angle) & 0xffffu);
    const int32_t direct = lookup(r, angle & 0xffffu);

    int32_t remainder;
    const int32_t q = gp4::idiv64(static_cast<int64_t>(complement << 14), static_cast<int32_t>(divisor), &remainder);

    const uint32_t outC = mulShift14(c, q);
    const uint32_t outB = mulShift14(b, q);
    const uint64_t wide = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(q));
    const uint32_t previous = static_cast<uint32_t>(wide >> 13);
    const uint32_t outA = static_cast<uint32_t>(wide >> 14);

    const uint32_t flags = (previous & 1u)
        | parityFlag(outA)
        | (outA == 0u ? 0x40u : 0u)
        | ((outA & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ outA) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = static_cast<uint32_t>(direct);
    r->ebx = outA;
    r->ecx = outB;
    r->edx = outC;
    r->ebp = static_cast<uint32_t>(q);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e46e, FUN_0040e46e, "regs(eax:i32, ebx:i32, ecx:i32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of)")
