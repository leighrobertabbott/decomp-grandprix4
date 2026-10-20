#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: draws the string at 0x00823200 through the text-output routine 0x0054d211 (esi = the
// string). popal restores every register; the flags are those 0x0054d211 leaves.
// FUNCTION: GP4 0x0052ac82
void __cdecl FUN_0052ac82(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.esi = 0x00823200u;
    gp4::call_regs(0x0054d211, call);
    r->eflags = gp4::flags::merge(r->eflags, call.eflags);
}
GP4_IMPL(0x0052ac82, FUN_0052ac82, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=textout_0054d211")
