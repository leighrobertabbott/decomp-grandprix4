#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;
using gp4::x87::ext_i32;

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00411c75
// DAT_006f2bf4 = eax = round(|a * (c1 * a + c2)| * K2 * scale[ecx] * K3) with a = -(double)eax * K1,
// (c1, c2) the coefficient pair at +8/+0x10 of the double table 0x0062f4a0 (ecx < 2) or 0x0062f6c8,
// scale = int table at 0x0062f870. The whole chain stays in 80-bit registers up to the FISTP.
void __cdecl FUN_00411c75(gp4::Regs* r) {
    const uint32_t input = r->eax;
    const uint32_t index = r->ecx;

    // mov [0x6f2bf4], eax / fild dword [0x6f2bf4] / fchs / fmul qword [0x601440]
    const Ext a = gp4::x87::fchs(ext_i32(static_cast<int32_t>(input))) * ext(GP4_GLOBAL(double, 0x00601440));

    const bool upper = static_cast<int32_t>(index) >= 2;      // cmp ecx, 2 / jl
    const double* table = upper ? GP4_ARRAY(const double, 0x0062f6c8) : GP4_ARRAY(const double, 0x0062f4a0);

    const Ext poly = ext(table[1]) * a + ext(table[2]);        // fld [eax+8] / fmul st(1) / fadd [eax+0x10]
    const Ext mag = gp4::x87::fabs(a * poly);                  // fmulp st(1) / fabs
    const Ext scaled = mag * ext(GP4_GLOBAL(double, 0x00601460));
    const Ext weighted = scaled * ext_i32(GP4_ARRAY(const int32_t, 0x0062f870)[index]);   // fild [ecx*4+0x62f870] / fmulp
    const int32_t result = gp4::x87::fistp32(weighted * ext(GP4_GLOBAL(double, 0x006f2b9c)));

    GP4_GLOBAL(int32_t, 0x006f2bf4) = result;
    r->eax = static_cast<uint32_t>(result);
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(index, 2u);
}
GP4_IMPL(0x00411c75, FUN_00411c75, "regs(eax:u32, ecx:u32[0..3]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
