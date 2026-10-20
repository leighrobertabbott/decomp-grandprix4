#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Scales |eax| by DAT_00602a4c / 64 / DAT_00602a40, converts the quotient to thousandths against
// DAT_0062ee04 (the divisor is kept above the high word so the second division cannot overflow),
// clamps the magnitude to 20000 and restores the sign of the input. ecx and edx are preserved;
// the flags come from the final "test edx, edx" / "neg eax".
// FUNCTION: GP4 0x00432372
void __cdecl FUN_00432372(gp4::Regs* r) {
    const uint32_t input = r->eax;
    const bool negative = static_cast<int32_t>(input) < 0;
    const uint32_t magnitude = negative ? 0u - input : input;

    const uint64_t scaled = (static_cast<uint64_t>(magnitude) * GP4_GLOBAL(uint32_t, 0x00602a4c)) >> 6;
    uint32_t remainder;
    const uint32_t ratio = gp4::div64(scaled, GP4_GLOBAL(uint32_t, 0x00602a40), &remainder);

    const uint64_t thousandths = static_cast<uint64_t>(ratio) * 0x3e8u;
    uint32_t high = static_cast<uint32_t>(thousandths >> 32);
    const uint32_t low = static_cast<uint32_t>(thousandths);
    uint32_t divisor = GP4_GLOBAL(uint32_t, 0x0062ee04);
    if (!(divisor > high)) {
        divisor = high + 1u;
        if (divisor == 0u) {
            divisor = 0xffffffffu;
            high = high - 1u;
        }
    }
    uint32_t value = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &remainder);
    if (value > 0x4e20u) value = 0x4e20u;

    uint32_t flags;
    if (negative) {
        const uint32_t result = 0u - value;
        // neg: CF = source != 0, OF = source == 0x80000000
        flags = (value != 0u ? 1u : 0u)
            | parityFlag(result)
            | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (value == 0x80000000u ? 0x800u : 0u);
        value = result;
    } else {
        // test edx, edx on the original eax (non-negative): CF = OF = AF = 0
        flags = parityFlag(input) | (input == 0u ? 0x40u : 0u);
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432372, FUN_00432372, "regs(eax:i32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00432372")
