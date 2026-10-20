#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs the device-slot routine 0x0051d1d0 with the selector `eax & 0xf`, ebx = 1,
// ebp = 1, edx = 0, then popal restores every register. Only the flags left by the callee survive.
// FUNCTION: GP4 0x0051d49a
void __cdecl FUN_0051d49a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = r->eax & 0xfu;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(c.eax));
    c.ebx = 1u;
    c.ebp = 1u;
    c.edx = 0u;
    gp4::call_regs(0x0051d1d0, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051d49a, FUN_0051d49a, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
