#include <gp4/gp4.h>

struct Unknown_0041ae6c {
    uint8_t Unknown00[0xa0];
    uint16_t UnknownA0;
    uint8_t UnknownA2[0x1ae - 0xa2];
    uint16_t Unknown1AE;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp/sub left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Consumes `step` (edx) from the counter at +0x1ae; when the counter runs out, the
// remainder fraction (in 1/0x4000 units) of the step is stored at +0xa0.
// FUNCTION: GP4 0x0041ae6c
void __cdecl FUN_0041ae6c(gp4::Regs* r) {
    Unknown_0041ae6c* body = reinterpret_cast<Unknown_0041ae6c*>(r->esi);
    const uint32_t step = r->edx;
    const uint32_t counter = body->Unknown1AE;
    uint32_t flags;

    if ((int32_t)counter < (int32_t)step) {
        body->Unknown1AE = 0;
        const uint32_t numerator = (step - counter) << 14;
        const uint32_t quotient = numerator / step;
        const uint32_t remainder = numerator % step;
        body->UnknownA0 = (uint16_t)quotient;
        r->ebp = step;
        r->eax = quotient;
        r->edx = remainder;
        flags = 0x44u;   // xor edx, edx; div leaves them
    } else {
        const uint32_t left = counter - step;
        body->Unknown1AE = (uint16_t)left;
        body->UnknownA0 = 0;
        r->eax = left;
        flags = subFlags(counter, step);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041ae6c, FUN_0041ae6c, "regs(edx:i32, esi:ptr[0x1b6]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of)")
