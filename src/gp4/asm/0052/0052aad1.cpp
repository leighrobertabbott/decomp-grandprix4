#include <gp4/gp4.h>

// pushal wrapper that sets 0x00827559 twice through 0x00531af4 (first 0x3ff, then the incoming eax;
// the (empty) routines 0x00531b03 / 0x00531b2e are called in between with the rect-corner
// registers prepared in ebx/ebp/eax/ecx). Every GPR is restored by popal; the flags are those the
// last 0x00531af4 call leaves (`and eax, 0xffff` of the incoming eax).
// FUNCTION: GP4 0x0052aad1
void __cdecl FUN_0052aad1(gp4::Regs* r) {
    const uint32_t value = r->eax;

    gp4::cdecl_<0x00531b03, void>();

    gp4::Regs first = {};
    first.eax = 0x3ffu;
    first.eflags = r->eflags;
    gp4::call_regs(0x00531af4, first);

    gp4::cdecl_<0x00531b2e, void>();

    gp4::Regs second = {};
    second.eax = value;
    second.eflags = first.eflags;
    gp4::call_regs(0x00531af4, second);

    r->eflags = second.eflags;
}
GP4_IMPL(0x0052aad1, FUN_0052aad1, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
