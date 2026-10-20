#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;
using gp4::x87::narrow;

static inline double& Field(uint8_t* rec, uint32_t off) { return *reinterpret_cast<double*>(rec + off); }

// exp(-(x*k)) the way the original evaluates it: y = log2(e) * -(x*k), split into
// 2^n * 2^(y-n) with FRNDINT (and the FSUB/FSCALE after it) under the control word
// at 0x6f29a4, then restoring the control word at 0x6f29a6.
static inline Ext ExpNeg(const Ext& x, double k) {
    const Ext y = gp4::x87::fchs(x * ext(k)) * gp4::x87::ext_l2e();
    gp4::x87::set_cw(GP4_GLOBAL(uint16_t, 0x006f29a4));
    const Ext n = gp4::x87::frndint(y);
    const Ext f = y - n;
    const Ext p = gp4::x87::fscale(gp4::x87::ext_one(), n);
    gp4::x87::set_cw(GP4_GLOBAL(uint16_t, 0x006f29a6));
    return p * (gp4::x87::f2xm1(f) + gp4::x87::ext_one());
}

// FUNCTION: GP4 0x00411a22
void __cdecl FUN_00411a22(gp4::Regs* r) {
    const uint32_t index = r->ecx;
    const Ext x = gp4::x87::in_st(r, 0);
    const bool upper = static_cast<int32_t>(index) >= 2;
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x00601510) + index * 0x158u;

    Field(rec, 0x70) = narrow(x);

    const double* t = upper ? GP4_ARRAY(const double, 0x0062f638) : GP4_ARRAY(const double, 0x0062f410);
    Field(rec, 0xd8) = narrow((ext(t[1]) * x + ext(t[2])) * x * ext(Field(rec, 0x08)));
    {
        const Ext s = gp4::x87::fsin(gp4::x87::fpatan(x, ext(t[4])) * ext(GP4_GLOBAL(double, 0x006f2a64)));
        Field(rec, 0xc8) = narrow((s * ext(Field(rec, 0x10))) / (ext(Field(rec, 0xd8)) * ext(Field(rec, 0xd0))));
    }
    Field(rec, 0x78) = narrow(ext(t[6]) * x + ext(t[7]));
    Field(rec, 0xe8) = narrow(ext(t[8]) * x + ext(t[9]) + ext(Field(rec, 0x20)));
    Field(rec, 0xf0) = narrow(((ext(Field(rec, 0x28)) * x + ext(Field(rec, 0x30))) + ext(t[0xb])) * x + ext(t[0xc]));

    t = upper ? GP4_ARRAY(const double, 0x0062f738) : GP4_ARRAY(const double, 0x0062f510);
    Field(rec, 0x108) = narrow((x * ext(t[1]) + ext(t[2])) * x * ext(Field(rec, 0x38)));
    {
        const Ext p = (x * ext(t[3]) + ext(t[4])) * x * ext(Field(rec, 0x40));
        const Ext q = p * ExpNeg(x, t[5]);
        Field(rec, 0xf8) = narrow(q / (ext(Field(rec, 0x108)) * ext(Field(rec, 0x100))));
    }
    Field(rec, 0x80) = narrow((ext(t[7]) * x + ext(t[8])) * x + ext(t[9]));
    Field(rec, 0x118) = narrow(ext(t[0xb]) * x + ext(t[0xc]) + ext(Field(rec, 0x58)));
    Field(rec, 0x120) = narrow(((ext(Field(rec, 0x60)) * x + ext(Field(rec, 0x68))) + ext(t[0xe])) * x + ext(t[0xf]));

    t = upper ? GP4_ARRAY(const double, 0x0062f6c8) : GP4_ARRAY(const double, 0x0062f4a0);
    Field(rec, 0x138) = narrow((ext(t[1]) * x + ext(t[2])) * x);
    {
        const Ext p = (x * ext(t[3]) + ext(t[4])) * x;
        const Ext q = p * ExpNeg(x, t[5]);
        Field(rec, 0x128) = narrow(q / (ext(Field(rec, 0x138)) * ext(Field(rec, 0x130))));
    }
    Field(rec, 0x88) = narrow((ext(t[6]) * x + ext(t[7])) * x + ext(t[8]));
    Field(rec, 0x148) = narrow(ext(t[9]) * x + ext(t[10]));
    Field(rec, 0x150) = narrow(ext(t[11]) * x + ext(t[12]));
    Field(rec, 0x90) = narrow(ext(t[13]));

    t = upper ? GP4_ARRAY(const double, 0x0062f808) : GP4_ARRAY(const double, 0x0062f5e0);
    Field(rec, 0x98) = narrow((gp4::x87::fchs(ext(t[7]) * x + ext(t[8])) + ext(Field(rec, 0x00))) * x * ext(t[6]));

    // registers and flags as the original leaves them: eax = record, edx = last table,
    // flags of the final `cmp ecx, 2`
    const uint32_t res = index - 2u;
    uint32_t parity = res & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    uint32_t fl = r->eflags & ~0x8d5u;
    if (index < 2u) fl |= 0x001u;
    if ((parity & 1u) == 0) fl |= 0x004u;
    if ((index ^ 2u ^ res) & 0x10u) fl |= 0x010u;
    if (res == 0) fl |= 0x040u;
    if (res & 0x80000000u) fl |= 0x080u;
    if ((index ^ 2u) & (index ^ res) & 0x80000000u) fl |= 0x800u;
    r->eflags = fl;
    r->eax = 0x00601510u + index * 0x158u;
    r->edx = reinterpret_cast<uint32_t>(t);
}
GP4_IMPL(0x00411a22, FUN_00411a22, "regs(ecx:u32[0..3], st0:f64) -> (eax, edx, cf, pf, af, zf, sf, of)")
