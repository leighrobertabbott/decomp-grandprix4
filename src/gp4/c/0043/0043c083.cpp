#include <gp4/gp4.h>

// After 0x0043bf98, forms for each of the six selected curves (rows of 12 dwords at 0x602dec chosen by
// [0x62d1d8 + 4*i], 0x30 bytes apart) the dot product with the twelve weights at 0x63b8b0, each term
// (value * weight) >> 14 as a 64-bit product (imul / shrd 14), and stores the sums at 0x63b844 + 4*i.
// 0x0040bb26 then runs with the registers the loops leave (ecx = 6, esi = 12, ebx = last row offset,
// ebp = last sum); pushal / popal restores every register and leaves the flags of 0x0040bb26.
// FUNCTION: GP4 0x0043c083
void __cdecl FUN_0043c083(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0043bf98, c);

    uint32_t ebx = 0;
    uint32_t sum = 0;
    uint32_t low = 0;
    uint32_t high = 0;
    for (uint32_t row = 0; row != 6u; ++row) {
        ebx = GP4_ARRAY(uint32_t, 0x0062d1d8)[row] * 0x30u;
        sum = 0;
        for (uint32_t col = 0; col != 12u; ++col) {
            const int32_t value = *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x00602decu + ebx + col * 4u));
            const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_ARRAY(int32_t, 0x0063b8b0)[col]);
            high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            low = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
            sum += low;
        }
        GP4_ARRAY(uint32_t, 0x0063b844)[row] = sum;
    }

    c.eax = low;
    c.ebx = ebx;
    c.ecx = 6u;
    c.edx = high;
    c.esi = 12u;
    c.ebp = sum;
    c.eflags = (c.eflags & ~0x8d5u) | 0x44u;
    gp4::call_regs(0x0040bb26, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043c083, FUN_0043c083, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0040c0ae+lim_00937dd0+state_0043c1ba+state_0040b99a+map_0040c43b+state_0040bb26")
