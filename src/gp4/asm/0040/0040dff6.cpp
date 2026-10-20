#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (value * factor) >> 14 as `imul; shrd eax, edx, 14` leaves it: the low 32 bits of the signed 64-bit
// product shifted right by 14.
static inline uint32_t mulQ14(uint32_t value, uint32_t factor) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value)) * static_cast<int64_t>(static_cast<int32_t>(factor));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Multiplies the quaternion (g0, g1, g2, g3) = (DAT_00625a8c, a90, a94, a98) (Q14) by the quaternion
// (eax, ebx, ecx, edx) = (a, b, c, d) and returns the product in (eax, ebx, ecx, edx); ebp is left holding
// the last component and the flags are those of the final add. esi and edi are restored.
//   eax = a*g0 - b*g1 - c*g2 - d*g3     ebx = a*g1 + b*g0 + c*g3 - d*g2
//   ecx = a*g2 - b*g3 + c*g0 + d*g1     edx = a*g3 + b*g2 - c*g1 + d*g0
// FUNCTION: GP4 0x0040dff6
void __cdecl FUN_0040dff6(gp4::Regs* r) {
    const uint32_t a = r->eax;
    const uint32_t b = r->ebx;
    const uint32_t c = r->ecx;
    const uint32_t d = r->edx;
    const uint32_t g0 = GP4_GLOBAL(uint32_t, 0x00625a8c);
    const uint32_t g1 = GP4_GLOBAL(uint32_t, 0x00625a90);
    const uint32_t g2 = GP4_GLOBAL(uint32_t, 0x00625a94);
    const uint32_t g3 = GP4_GLOBAL(uint32_t, 0x00625a98);

    const uint32_t first = mulQ14(a, g0) - mulQ14(b, g1) - mulQ14(c, g2) - mulQ14(d, g3);
    const uint32_t second = mulQ14(a, g1) + mulQ14(b, g0) + mulQ14(c, g3) - mulQ14(d, g2);
    const uint32_t third = mulQ14(a, g2) - mulQ14(b, g3) + mulQ14(c, g0) + mulQ14(d, g1);
    const uint32_t partial = mulQ14(a, g3) + mulQ14(b, g2) - mulQ14(c, g1);
    const uint32_t last = mulQ14(d, g0);
    const uint32_t fourth = partial + last;

    r->eax = first;
    r->ebx = second;
    r->ecx = third;
    r->edx = fourth;
    r->ebp = fourth;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(partial, last);
}
GP4_IMPL(0x0040dff6, FUN_0040dff6, "regs(eax:i32, ebx:i32, ecx:i32, edx:i32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
