#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// ZF, SF, PF of a 32-bit result.
static inline uint32_t resultFlags(uint32_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | (res & 0x80000000u ? 0x80u : 0u);
}

// The Q14 polynomial both paths share: with m the argument and Q14 one = 0x4000,
//   r = (((((m >> 2) + 1) * m / 0xc000 + 1) * m >> 14) >> 1) + 1) * m >> 14) + 1
// (every product a signed 64-bit imul, shifts are SHRD / SAR on the low half).
static inline int32_t polynomialQ14(int32_t m) {
    const int64_t p0 = static_cast<int64_t>((m >> 2) + 0x4000) * m;
    int32_t rem;
    int32_t acc = gp4::idiv64(p0, 0xc000, &rem) + 0x4000;
    const int64_t p1 = static_cast<int64_t>(acc) * m;
    acc = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14)) >> 1;
    acc += 0x4000;
    const int64_t p2 = static_cast<int64_t>(acc) * m;
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(p2) >> 14));
}

// Q14 series evaluated at the angle-like argument in eax: for |eax| < 0x1000 the polynomial of eax, otherwise
// the polynomial of eax & 0xfff times the table entry [0x623cbc + 4*(clamp(eax >> 12, -40, 31) + 40)].
// ebp/ecx/edx are saved and restored; the result is left in eax.
// FUNCTION: GP4 0x00401bc2
void __cdecl FUN_00401bc2(gp4::Regs* r) {
    const int32_t arg = static_cast<int32_t>(r->eax);
    const uint32_t magnitude = arg < 0 ? 0u - static_cast<uint32_t>(arg) : static_cast<uint32_t>(arg);

    uint32_t result;
    uint32_t flags;
    if (magnitude < 0x1000u) {
        const uint32_t before = static_cast<uint32_t>(polynomialQ14(arg));
        result = before + 0x4000u;
        // add eax, 0x4000
        flags = resultFlags(result);
        if (result < before) flags |= 0x01u;
        if (((before ^ 0x4000u ^ result) & 0x10u) != 0u) flags |= 0x10u;
        if (((before ^ result) & (0x4000u ^ result) & 0x80000000u) != 0u) flags |= 0x800u;
    } else {
        int32_t index = (arg & static_cast<int32_t>(0xfffff000)) >> 12;
        if (index > 0x1f) index = 0x1f;
        if (index < -0x28) index = -0x28;
        index += 0x28;
        const int32_t m = arg & 0xfff;
        const int32_t poly = polynomialQ14(m) + 0x4000;
        const int32_t entry = GP4_ARRAY(int32_t, 0x00623cbc)[index];
        const int64_t product = static_cast<int64_t>(poly) * entry;
        result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        // shrd eax, edx, 14: CF = last bit out, AF = 0 (pinned model), OF = bit31(prev ^ result)
        const uint32_t prev = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
        flags = resultFlags(result);
        if ((prev & 1u) != 0u) flags |= 0x01u;
        if (((prev ^ result) & 0x80000000u) != 0u) flags |= 0x800u;
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00401bc2, FUN_00401bc2, "regs(eax:i32[-0x40000..0x40000]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
