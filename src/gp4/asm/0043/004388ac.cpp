#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// (value << 14) as the edx:eax pair "cdq / shld edx,eax,14 / shl eax,14" builds, divided by the length.
static inline int32_t scaledQuotient(int32_t value, int32_t length, int32_t* remainder) {
    const uint32_t low = static_cast<uint32_t>(value) << 14;
    const uint32_t high = static_cast<uint32_t>(value >> 18);
    return gp4::idiv64(static_cast<int64_t>((static_cast<uint64_t>(high) << 32) | low), length, remainder);
}

// Normalises the vector (eax, ebx, ecx) to a 14-bit fixed-point unit vector: the length is the
// integer square root (0x0040113f) of the 64-bit sum of squares, at least 1, and each component
// becomes (component << 14) / length (signed 64/32 division). ebp leaves holding the length, edx
// the remainder of the last division; the flags are those of the final "shl eax, 0xe" (applied to
// the original ecx), since idiv leaves them untouched.
// FUNCTION: GP4 0x004388ac
void __cdecl FUN_004388ac(gp4::Regs* r) {
    const int32_t x = static_cast<int32_t>(r->eax);
    const int32_t y = static_cast<int32_t>(r->ebx);
    const int32_t z = static_cast<int32_t>(r->ecx);

    const uint64_t sumOfSquares = static_cast<uint64_t>(static_cast<int64_t>(x) * static_cast<int64_t>(x))
        + static_cast<uint64_t>(static_cast<int64_t>(y) * static_cast<int64_t>(y))
        + static_cast<uint64_t>(static_cast<int64_t>(z) * static_cast<int64_t>(z));

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(sumOfSquares);
    call.edx = static_cast<uint32_t>(sumOfSquares >> 32);
    gp4::call_regs(0x0040113f, call);

    int32_t length = static_cast<int32_t>(call.eax);
    if (length < 1) length = 1;

    int32_t remainder;
    const int32_t quotientX = scaledQuotient(x, length, &remainder);
    const int32_t quotientY = scaledQuotient(y, length, &remainder);
    const int32_t quotientZ = scaledQuotient(z, length, &remainder);

    // shl eax, 14 on the original ecx: CF = last bit out, AF = 0, OF = CF xor the new sign bit.
    const uint32_t shifted = static_cast<uint32_t>(z) << 14;
    const uint32_t shiftCarry = (static_cast<uint32_t>(z) >> 18) & 1u;
    const uint32_t flags = shiftCarry
        | parityFlag8(shifted)
        | (shifted == 0u ? 0x40u : 0u)
        | ((shifted >> 24) & 0x80u)
        | ((shiftCarry ^ (shifted >> 31)) != 0u ? 0x800u : 0u);

    r->eax = static_cast<uint32_t>(quotientX);
    r->ebx = static_cast<uint32_t>(quotientY);
    r->ecx = static_cast<uint32_t>(quotientZ);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = static_cast<uint32_t>(length);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004388ac, FUN_004388ac, "regs(eax:i32[-0x1000000..0x1000000], ebx:i32[-0x1000000..0x1000000], ecx:i32[-0x1000000..0x1000000]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of)")
