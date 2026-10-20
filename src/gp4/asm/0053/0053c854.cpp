#include <gp4/gp4.h>
#include <gp4/flags.h>

namespace {

// Flags of `adc value, 1` with carry-in `cin`.
static inline uint32_t adc_one_flags(uint32_t a, uint32_t cin, uint32_t* result) {
    const uint32_t b = 1u;
    const uint32_t res = a + b + cin;
    const uint32_t carry = static_cast<uint32_t>((static_cast<uint64_t>(a) + b + cin) >> 32);
    *result = res;
    return (carry ? gp4::flags::CF : 0u) | gp4::flags::szp(res, 32)
         | (((a ^ b ^ res) & 0x10u) ? gp4::flags::AF : 0u)
         | (((~(a ^ b) & (a ^ res)) & 0x80000000u) ? gp4::flags::OF : 0u);
}

}  // namespace

// Signed 64 x 32 multiply wrapper around 0x0053c835: the sign of the edx:eax operand (negated as a
// 64-bit pair, with the original's `adc edx, 1`) and of ebx (negated to its magnitude) is tracked in ecx
// (+1 / -1, saved and restored); the product pair returned by the helper is negated again when the signs
// differed. ebx comes back as |ebx|. The flags are those of the final `or ecx, ecx` or `adc edx, 1`.
// FUNCTION: GP4 0x0053c854
void __cdecl FUN_0053c854(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebx = r->ebx;
    int32_t sign = 1;

    if (static_cast<int32_t>(edx) < 0) {
        edx = ~edx;
        eax = ~eax;
        const uint32_t carry = (eax == 0xffffffffu) ? 1u : 0u;  // add eax, 1
        eax += 1u;
        edx = edx + 1u + carry;                                   // adc edx, 1
        sign = -sign;
    }
    if (static_cast<int32_t>(ebx) < 0) {
        ebx = 0u - ebx;
        sign = -sign;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = eax;
    c.edx = edx;
    c.ebx = ebx;
    gp4::call_regs(0x0053c835, c);
    eax = c.eax;
    edx = c.edx;

    uint32_t arith = gp4::flags::logic32(static_cast<uint32_t>(sign));
    if (sign < 0) {
        edx = ~edx;
        eax = ~eax;
        const uint32_t carry = (eax == 0xffffffffu) ? 1u : 0u;  // add eax, 1
        eax += 1u;
        arith = adc_one_flags(edx, carry, &edx);
    }

    r->eax = eax;
    r->edx = edx;
    r->ebx = ebx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053c854, FUN_0053c854, "regs(eax:u32, edx:u32, ebx:u32) -> (eax, edx, ebx, cf, pf, af, zf, sf, of)")
