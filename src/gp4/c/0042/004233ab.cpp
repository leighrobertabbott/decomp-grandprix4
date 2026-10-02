#include <gp4/gp4.h>

static inline uint32_t logicalFlags(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004233ab
void __cdecl FUN_004233ab(gp4::Regs* r) {
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x00609840);
    if (divisor != 0) {
        const int32_t first = GP4_GLOBAL(int32_t, 0x00609834);
        const int32_t second = GP4_GLOBAL(int32_t, 0x00609838);
        const int32_t third = GP4_GLOBAL(int32_t, 0x0060983c);
        GP4_GLOBAL(int32_t, 0x00609844) = first / divisor;
        GP4_GLOBAL(int32_t, 0x00609848) = second / divisor;
        const int32_t quotient = third / divisor;
        const int32_t remainder = third % divisor;
        GP4_GLOBAL(int32_t, 0x0060984c) = quotient;
        r->eax = static_cast<uint32_t>(quotient);
        r->edx = static_cast<uint32_t>(remainder);
    }
    r->ebp = static_cast<uint32_t>(divisor);
    r->eflags = (r->eflags & ~0x8d5u) | logicalFlags(static_cast<uint32_t>(divisor));
}
GP4_IMPL(0x004233ab, FUN_004233ab, "regs(eax:i32, edx:i32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=divide_00609834")
