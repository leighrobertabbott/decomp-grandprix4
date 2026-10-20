#include <gp4/gp4.h>
#include <gp4/flags.h>

// 64-bit by 32-bit multiply helper: the full product eax * ebx is stored in the pair
// [0x0082fd28] (low) / [0x0082fd2c] (high) and returned as eax = low. The incoming edx is
// multiplied by ebx as well, and its low word is added to the stored high word to give the
// returned edx (the cross term of a signed 64 x 32 multiply). The flags are those of the final
// `add edx, [0x0082fd2c]`; the first mul's flags are overwritten by the later writers.
// FUNCTION: GP4 0x0053c835
void __cdecl FUN_0053c835(gp4::Regs* r) {
    const uint32_t eax_in = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t edx_in = r->edx;

    const uint64_t product = static_cast<uint64_t>(eax_in) * ebx;
    GP4_GLOBAL(uint32_t, 0x0082fd28) = static_cast<uint32_t>(product);
    GP4_GLOBAL(uint32_t, 0x0082fd2c) = static_cast<uint32_t>(product >> 32);

    const uint64_t cross = static_cast<uint64_t>(edx_in) * ebx;
    const uint32_t cross_lo = static_cast<uint32_t>(cross);
    const uint32_t high = GP4_GLOBAL(uint32_t, 0x0082fd2c);
    const uint32_t arith = gp4::flags::add32(cross_lo, high);

    r->edx = cross_lo + high;
    r->eax = GP4_GLOBAL(uint32_t, 0x0082fd28);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053c835, FUN_0053c835, "regs(eax:u32, ebx:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of)")
