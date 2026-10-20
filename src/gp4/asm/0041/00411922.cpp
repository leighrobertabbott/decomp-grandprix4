#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Fills the 0x158-byte coefficient record number ecx at 0x00601510 from the value x on the x87
// stack (consumed) and three 0x78+ qword parameter tables chosen by ecx < 2 (signed): one set
// at 0x0062f410 / 0x0062f510 / 0x0062f4a0, the other at 0x0062f638 / 0x0062f738 / 0x0062f6c8.
// The stack value x stays in 80 bits through every product and sum; x*x is formed once from the
// 80-bit value. eax = the record address, edx = the last table, flags those of `cmp ecx, 2`.
// FUNCTION: GP4 0x00411922
void __cdecl FUN_00411922(gp4::Regs* r) {
    using namespace gp4::x87;

    const uint32_t index = r->ecx;
    const Ext x = in_st(r, 0);
    const Ext one = ext_one();

    double* rec = reinterpret_cast<double*>(static_cast<uintptr_t>(0x00601510u + 0x158u * index));
    const bool low = static_cast<int32_t>(index) < 2;

    rec[0] = narrow(x);
    const Ext x2 = x * x;

    const double* a = reinterpret_cast<const double*>(static_cast<uintptr_t>(low ? 0x0062f410u : 0x0062f638u));
    rec[26] = narrow(ext(a[0]));
    rec[1] = narrow(fchs(ext(a[15]) * x2) + one);
    rec[2] = narrow((fchs(fabs(x) * ext(a[5])) + one) * ext(a[3]));
    rec[3] = narrow(x * ext(a[16]) + ext(a[17]));
    rec[4] = narrow(x * ext(a[10]));
    rec[5] = narrow(x * ext(a[13]));
    rec[6] = narrow(x * ext(a[14]));

    const double* b = reinterpret_cast<const double*>(static_cast<uintptr_t>(low ? 0x0062f510u : 0x0062f738u));
    rec[32] = narrow(ext(b[0]));
    rec[7] = narrow(fchs(x2 * ext(b[18])) + one);
    rec[8] = narrow(fchs(fabs(x) * ext(b[6])) + one);
    rec[9] = narrow(x * ext(b[19]) + ext(b[20]));
    rec[10] = narrow(fchs(fabs(x) * ext(b[10])) + one);
    rec[11] = narrow(x * ext(b[13]));
    rec[12] = narrow(x * ext(b[16]));
    rec[13] = narrow(x * ext(b[17]));

    const double* c = reinterpret_cast<const double*>(static_cast<uintptr_t>(low ? 0x0062f4a0u : 0x0062f6c8u));
    rec[38] = narrow(ext(c[0]));

    r->eax = 0x00601510u + 0x158u * index;
    r->edx = low ? 0x0062f4a0u : 0x0062f6c8u;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags(index, 2u);
}
GP4_IMPL(0x00411922, FUN_00411922, "regs(ecx:u32[0..3], st0:f64) -> (eax, edx, cf, pf, af, zf, sf, of)")
