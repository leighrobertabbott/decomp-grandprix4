#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal wrapper: calls 0x004feb50(count = [0x0083055b], table = 0x008300db) (stdcall, two
// arguments; the original pushes the table first, then the count). popal restores every register; the
// flags are those the callee leaves.
// FUNCTION: GP4 0x0053ef3f
void __cdecl FUN_0053ef3f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t args[2] = {GP4_GLOBAL(uint32_t, 0x0083055b), 0x008300dbu};
    gp4::call_regs_stdcall(0x004feb50, c, args);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053ef3f, FUN_0053ef3f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846")
