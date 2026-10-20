#include <gp4/gp4.h>
#include <gp4/x87.h>

// Converts the rotation vector (x, y, z) at 0x622cb0 (|v| = s) into a quaternion stored at
// [edx + 0x622c00]: (cos(s * k), x * sin(s * k) / s, y * ..., z * ...), with k = [0x6f2a54];
// the identity (1, 0, 0, 0) when s == 0. All intermediates stay in extended precision, like the
// original's x87 stack. The routine ends in fnstsw ax / sahf on the ftst of s: eax's low word is
// the status word (TOP = 7 gives 0x3800, plus C3|C2|C0), and CF = C0, PF = C2, AF = status bit 12
// (always set), ZF = C3 are loaded from it; SF = 0 and OF is untouched.
// FUNCTION: GP4 0x00402848
void __cdecl FUN_00402848(gp4::Regs* r) {
    using namespace gp4::x87;

    double* out = reinterpret_cast<double*>(static_cast<uintptr_t>(0x00622c00u + r->edx));

    const Ext x = ext(GP4_GLOBAL(double, 0x00622cb0));
    Ext sum = x * x;
    const Ext y = ext(GP4_GLOBAL(double, 0x00622cb8));
    sum = sum + y * y;
    const Ext z = ext(GP4_GLOBAL(double, 0x00622cc0));
    sum = sum + z * z;
    const Ext s = fsqrt(sum);

    const uint32_t status = 0x3800u | ftst(s);

    if ((status & 0x4000u) != 0u) {
        out[0] = narrow(ext_one());
        const Ext zero = ext_zero();
        out[1] = narrow(zero);
        out[2] = narrow(zero);
        out[3] = narrow(zero);
    } else {
        Ext sine;
        Ext cosine;
        fsincos(s * ext(GP4_GLOBAL(double, 0x006f2a54)), &sine, &cosine);
        out[0] = narrow(cosine);
        const Ext scale = sine / s;
        out[3] = narrow(ext(GP4_GLOBAL(double, 0x00622cc0)) * scale);
        out[2] = narrow(ext(GP4_GLOBAL(double, 0x00622cb8)) * scale);
        out[1] = narrow(scale * ext(GP4_GLOBAL(double, 0x00622cb0)));
    }

    r->eax = (r->eax & 0xffff0000u) | status;
    const uint32_t ah = status >> 8;
    r->eflags = (r->eflags & ~0xd5u) | (ah & 0xd5u);
}
GP4_IMPL(0x00402848, FUN_00402848, "regs(eax:u32, edx:u32[0..0x80]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:f64")
