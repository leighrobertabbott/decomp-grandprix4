#include <gp4/gp4.h>

// Angle of the vector (x, sqrt(0x10000000 - x*x)) for the signed value x in eax: the radicand is
// the low 32 bits of 0x10000000 - x*x, clamped at zero, its integer square root comes from
// 0x004010f3 and the angle from 0x00401315 (eax = x, edx = root). edx is restored (push/pop);
// the flags are those of the angle routine.
// FUNCTION: GP4 0x0040139d
void __cdecl FUN_0040139d(gp4::Regs* r) {
    const uint32_t x = r->eax;

    int32_t radicand = static_cast<int32_t>(0x10000000u - x * x);
    if (radicand < 0) {
        radicand = 0;
    }

    gp4::Regs root = *r;
    root.st_in = 0;
    root.st_out = 0;
    root.eax = static_cast<uint32_t>(radicand);
    gp4::call_regs(0x004010f3, root);

    gp4::Regs angle = root;
    angle.eax = x;
    angle.edx = root.eax;
    gp4::call_regs(0x00401315, angle);

    r->eax = angle.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (angle.eflags & 0x8d5u);
}
GP4_IMPL(0x0040139d, FUN_0040139d, "regs(eax:i32[-0x7fffffff..0x7fffffff]) -> (eax, cf, pf, af, zf, sf, of)")
