#include <gp4/gp4.h>

struct StepResult { uint32_t eax, edx, ebp, flags; };

// One fixed-point integration step: position += rate * scale >> 8, then the
// trapezoid midpoint times scale (<< 8 as a 64-bit value) is added to the
// fraction/whole pair with add/adc.
static inline StepResult Step(int32_t rate, uint32_t* position, uint32_t* fraction, uint32_t* whole) {
    const uint32_t previous = *position;
    const int64_t rateProduct = static_cast<int64_t>(rate) * GP4_GLOBAL(int32_t, 0x0060384c);
    *position = previous + static_cast<uint32_t>(static_cast<uint64_t>(rateProduct) >> 8);

    const uint32_t current = *position;
    const uint32_t midpoint = static_cast<uint32_t>(static_cast<int32_t>(current - previous) >> 1) + previous;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(midpoint)) * GP4_GLOBAL(int32_t, 0x0060384c);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t addHigh = (high << 8) | (low >> 24);
    const uint32_t addLow = low << 8;

    const uint32_t oldFraction = *fraction;
    const uint32_t newFraction = oldFraction + addLow;
    *fraction = newFraction;
    const uint32_t carry = newFraction < oldFraction ? 1u : 0u;

    const uint32_t oldWhole = *whole;
    const uint32_t newWhole = oldWhole + addHigh + carry;
    *whole = newWhole;

    // flags of the 32-bit adc
    uint32_t parity = newWhole & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    StepResult out;
    out.eax = addLow;
    out.edx = addHigh;
    out.ebp = previous;
    out.flags = static_cast<uint32_t>((static_cast<uint64_t>(oldWhole) + addHigh + carry) >> 32)
        | ((~parity & 1u) << 2)
        | ((oldWhole ^ addHigh ^ newWhole) & 0x10u)
        | (newWhole == 0u ? 0x40u : 0u)
        | ((newWhole >> 31) << 7)
        | ((((oldWhole ^ newWhole) & (addHigh ^ newWhole)) >> 31) << 11);
    return out;
}

// FUNCTION: GP4 0x0041a01a
void __cdecl FUN_0041a01a(gp4::Regs* r) {
    Step(GP4_GLOBAL(int32_t, 0x00637f60), &GP4_GLOBAL(uint32_t, 0x0063804c),
         &GP4_GLOBAL(uint32_t, 0x00638d54), &GP4_GLOBAL(uint32_t, 0x00638040));
    const StepResult last = Step(GP4_GLOBAL(int32_t, 0x00637f64), &GP4_GLOBAL(uint32_t, 0x00638054),
                                 &GP4_GLOBAL(uint32_t, 0x00638d5c), &GP4_GLOBAL(uint32_t, 0x00638048));
    r->eax = last.eax;
    r->edx = last.edx;
    r->ebp = last.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | last.flags;
}
GP4_IMPL(0x0041a01a, FUN_0041a01a, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
