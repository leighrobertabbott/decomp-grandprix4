#include <gp4/gp4.h>
#include <gp4/flags.h>

// Rebuilds the rate-dependent constants (0x00412604), stores the rate word read by 0x0041276c in
// 0x006f2e90 (the 16.16 scale factor) and applies it to the six signed words at 0x00852dd4 with
// 0x0051fc9d. Registers and flags on exit are those of 0x0051fc9d (eax, ebx, ecx, edx, flags) except
// ebp, which 0x00412604 leaves behind.
// FUNCTION: GP4 0x0051fcc6
void __cdecl FUN_0051fcc6(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00412604, c);
    c.eax = gp4::cdecl_<0x0041276c, uint32_t>();
    GP4_GLOBAL(uint32_t, 0x006f2e90) = c.eax;
    gp4::call_regs(0x0051fc9d, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051fcc6, FUN_0051fcc6, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004124ea+state_004125ce+rate_007ad8b8")
