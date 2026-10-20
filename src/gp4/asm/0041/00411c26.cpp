#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (lhs < rhs ? 1u : 0u) | ((~parity & 1u) << 2)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Evaluates |(t1 * x + t2) * x| * DAT_00601460 * table[index] * DAT_006f2b9c with
// x = -eax * DAT_00601440, everything on the x87 stack, and rounds it into DAT_006f2bf4.
// FUNCTION: GP4 0x00411c26
void __cdecl FUN_00411c26(gp4::Regs* r) {
    const int32_t value = (int32_t)r->eax;
    const uint32_t index = r->ecx;

    GP4_GLOBAL(int32_t, 0x006f2bf4) = value;
    const Ext x = gp4::x87::fchs(gp4::x87::ext_i32(value)) * ext(GP4_GLOBAL(double, 0x00601440));

    const double* t = ((int32_t)index < 2) ? GP4_ARRAY(const double, 0x0062f410)
                                           : GP4_ARRAY(const double, 0x0062f638);
    const Ext poly = (ext(t[1]) * x + ext(t[2])) * x;
    const Ext scaled = gp4::x87::fabs(poly) * ext(GP4_GLOBAL(double, 0x00601460));
    const Ext weighted = scaled * gp4::x87::ext_i32(GP4_ARRAY(int32_t, 0x0062f860)[index]);
    const int32_t result = gp4::x87::fistp32(weighted * ext(GP4_GLOBAL(double, 0x006f2b9c)));

    GP4_GLOBAL(int32_t, 0x006f2bf4) = result;
    r->eax = (uint32_t)result;
    r->eflags = (r->eflags & ~0x8d5u) | SubtractFlags(index, 2u, index - 2u);
}
GP4_IMPL(0x00411c26, FUN_00411c26, "regs(eax:u32[0..0xffff], ecx:u32[0..3]) -> (eax, cf, pf, af, zf, sf, of); globals=keep")
