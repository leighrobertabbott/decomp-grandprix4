#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scales 0x269300 * [0x0083137b] / [0x008310c4] (0 when the divisor is zero), shifts it right by 14
// and then, while it exceeds the threshold of the next {threshold, divisor} pair, divides both it
// and a second accumulator (starting at 0xfa0) by that pair's divisor. The pairs are constant data
// stored in the code stream at 0x00542c87 (8-byte entries); the tenth "threshold" is the first
// dword of the following routine (0x5eb70f60), larger than any shifted value (<= 0x3ffff), so the
// walk always stops by then. eax = the scaled value, ebx = the second accumulator, edx = remainder
// of the last division (the high half of the imul product when the divisor was zero and no step
// ran); esi is restored. The flags are those of the last `cmp eax, [esi]`.
// FUNCTION: GP4 0x00542c43
void __cdecl FUN_00542c43(gp4::Regs* r) {
    static const uint32_t kSteps[] = {
        100u, 2u, 100u, 2u, 100u, 2u, 250u, 5u, 100u, 2u, 250u, 5u, 100u, 2u, 250u, 5u,
        0u, 1u, 0x5eb70f60u, 0u,
    };
    const int64_t product = (int64_t)0x269300 * (int64_t)GP4_GLOBAL(int32_t, 0x0083137b);
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x008310c4);
    uint32_t eax = (uint32_t)product;
    int32_t edx = (int32_t)(product >> 32);
    if (divisor != 0) {
        int32_t rem;
        eax = (uint32_t)gp4::idiv64(product, divisor, &rem);
        edx = rem;
    } else {
        eax = 0u;
    }
    eax >>= 14;
    int32_t ebx = 0x0fa0;

    const uint32_t* entry = kSteps;
    uint32_t arith = gp4::flags::sub32(eax, entry[0]);
    while (eax > entry[0]) {
        const int32_t step = (int32_t)entry[1];
        int32_t rem;
        eax = (uint32_t)gp4::idiv64((int64_t)(int32_t)eax, step, &rem);
        ebx = gp4::idiv64((int64_t)ebx, step, &rem);
        edx = rem;
        entry += 2;
        arith = gp4::flags::sub32(eax, entry[0]);
    }

    r->eax = eax;
    r->ebx = (uint32_t)ebx;
    r->edx = (uint32_t)edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00542c43, FUN_00542c43, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bounds_00542c43")
