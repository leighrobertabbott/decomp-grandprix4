#include <gp4/gp4.h>

// Stores the four inputs (a, b, c, d) = (eax, ebx, ecx, edx) at 0x00625a8c.. and
// feeds the arctangent routine 0x00401315 with
//   eax = 2 * (b*d - a*c)           (imaginary part)
//   edx = (a*a - b*b) - (c*c - d*d) (real part)
// i.e. the angle of the product of two complex squares. ebx = a*a - b*b and
// ecx = c*c - d*d stay in place; the flags and eax are those left by the callee.
// FUNCTION: GP4 0x0040e418
void __cdecl FUN_0040e418(gp4::Regs* r) {
    const uint32_t a = r->eax;
    const uint32_t b = r->ebx;
    const uint32_t c = r->ecx;
    const uint32_t d = r->edx;

    GP4_GLOBAL(uint32_t, 0x00625a8c) = a;
    GP4_GLOBAL(uint32_t, 0x00625a90) = b;
    GP4_GLOBAL(uint32_t, 0x00625a94) = c;
    GP4_GLOBAL(uint32_t, 0x00625a98) = d;

    const uint32_t realLeft = a * a - b * b;
    const uint32_t realRight = c * c - d * d;
    const uint32_t imag = (b * d - a * c) << 1;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = imag;
    callee.ebx = realLeft;
    callee.ecx = realRight;
    callee.edx = realLeft - realRight;
    gp4::call_regs(0x00401315, callee);

    r->eax = callee.eax;
    r->ebx = realLeft;
    r->ecx = realRight;
    r->edx = realLeft - realRight;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0040e418, FUN_0040e418, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
