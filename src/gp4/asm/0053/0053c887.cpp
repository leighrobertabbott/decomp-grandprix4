#include <gp4/gp4.h>
#include <gp4/flags.h>

namespace {

// Flags of `adc a, b` with carry-in `cin`; *result receives the sum.
static inline uint32_t adc32_flags(uint32_t a, uint32_t b, uint32_t cin, uint32_t* result) {
    const uint32_t res = a + b + cin;
    const uint64_t wide = static_cast<uint64_t>(a) + b + cin;
    *result = res;
    return ((wide >> 32) ? gp4::flags::CF : 0u) | gp4::flags::szp(res, 32)
         | (((a ^ b ^ res) & 0x10u) ? gp4::flags::AF : 0u)
         | (((~(a ^ b) & (a ^ res)) & 0x80000000u) ? gp4::flags::OF : 0u);
}

}  // namespace

// Unsigned 64 / 64 bit division by shift and subtract: the dividend edx:eax is shifted left through
// the 64-bit remainder ebp:ebx 64 times and the divisor edi:esi is subtracted whenever it fits
// (restoring it otherwise), the quotient bits collecting in eax. On return edx:eax = quotient,
// ecx:ebx = remainder (ecx = the high half, ebp is saved and restored). The flags are those of the
// last step: `inc eax` after a successful subtraction (CF = 0 from the sbb), else of the restoring
// `adc ebp, edi`.
// FUNCTION: GP4 0x0053c887
void __cdecl FUN_0053c887(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    const uint32_t esi = r->esi;
    const uint32_t edi = r->edi;
    uint32_t ebx = 0u;
    uint32_t ebp = 0u;
    uint32_t flags = 0u;
    for (uint32_t step = 0x40u; step != 0u; --step) {
        // add eax, eax / adc edx, edx / adc ebx, ebx / adc ebp, ebp
        const uint32_t fromEax = eax >> 31;
        eax <<= 1;
        const uint32_t fromEdx = edx >> 31;
        edx = (edx << 1) | fromEax;
        const uint32_t fromEbx = ebx >> 31;
        ebx = (ebx << 1) | fromEdx;
        ebp = (ebp << 1) | fromEbx;

        // sub ebx, esi / sbb ebp, edi
        const uint32_t lowBorrow = ebx < esi ? 1u : 0u;
        const uint32_t lowDiff = ebx - esi;
        const bool borrow = static_cast<uint64_t>(ebp) < static_cast<uint64_t>(edi) + lowBorrow;
        const uint32_t highDiff = ebp - edi - lowBorrow;
        if (borrow) {
            // add ebx, esi / adc ebp, edi: put the divisor back
            const uint32_t lowCarry = (lowDiff + esi) < esi ? 1u : 0u;
            ebx = lowDiff + esi;
            flags = adc32_flags(highDiff, edi, lowCarry, &ebp);
        } else {
            ebx = lowDiff;
            ebp = highDiff;
            flags = gp4::flags::inc32(eax, 0u);
            eax += 1u;
        }
    }
    r->eax = eax;
    r->edx = edx;
    r->ebx = ebx;
    r->ecx = ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053c887, FUN_0053c887, "regs(eax:u32, edx:u32, esi:u32, edi:u32{0,0,1,2,0xffffffff}) -> (eax, edx, ebx, ecx, cf, pf, af, zf, sf, of)")
