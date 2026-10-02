#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of test: CF=OF=AF=0
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0041982f
// Six accumulators at 0x00623e84.. are divided by the divisor [0x006228a8] (0 counts as 1).
void __cdecl FUN_0041982f(gp4::Regs* r) {
    const int32_t divisor0 = GP4_GLOBAL(int32_t, 0x006228a8);
    const int32_t divisor = divisor0 == 0 ? 1 : divisor0;

    GP4_GLOBAL(int32_t, 0x00623ea0) = GP4_GLOBAL(int32_t, 0x00623e84) / divisor;
    GP4_GLOBAL(int32_t, 0x00623ea4) = GP4_GLOBAL(int32_t, 0x00623e88) / divisor;
    GP4_GLOBAL(int32_t, 0x00623ea8) = GP4_GLOBAL(int32_t, 0x00623e8c) / divisor;
    GP4_GLOBAL(int32_t, 0x00623eac) = GP4_GLOBAL(int32_t, 0x00623e90) / divisor;
    GP4_GLOBAL(int32_t, 0x00623eb0) = GP4_GLOBAL(int32_t, 0x00623e94) / divisor;

    const int32_t last = GP4_GLOBAL(int32_t, 0x00623e98);
    const int32_t quotient = last / divisor;
    GP4_GLOBAL(int32_t, 0x00623eb4) = quotient;

    r->ebp = (uint32_t)divisor;
    r->eax = (uint32_t)quotient;
    r->edx = (uint32_t)(last % divisor);
    // idiv leaves the flags of the initial "test ebp, ebp"
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags((uint32_t)divisor0);
}
GP4_IMPL(0x0041982f, FUN_0041982f, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
