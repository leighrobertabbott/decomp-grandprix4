#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal wrapper: calls 0x004feae0(count = [0x0083055b], table = 0x008300db) (stdcall, two
// arguments; the original pushes the table first, then the count). popal restores every register; the
// flags are those the callee leaves.
// FUNCTION: GP4 0x0053ef2c
void __cdecl FUN_0053ef2c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t args[2] = {GP4_GLOBAL(uint32_t, 0x0083055b), 0x008300dbu};
    gp4::call_regs_stdcall(0x004feae0, c, args);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053ef2c, FUN_0053ef2c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
