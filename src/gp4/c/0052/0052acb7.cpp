#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads the local clock through 0x0052accd and stores dx at 0x0081d360 and cx at 0x0081d362. pushal/popal
// keep every register; the flags are what the callee leaves.
// FUNCTION: GP4 0x0052acb7
void __cdecl FUN_0052acb7(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0052accd, c);
    GP4_GLOBAL(uint16_t, 0x0081d360) = static_cast<uint16_t>(c.edx);
    GP4_GLOBAL(uint16_t, 0x0081d362) = static_cast<uint16_t>(c.ecx);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052acb7, FUN_0052acb7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
