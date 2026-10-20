#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; looks up the car-table record for the index at 0x00a69878 with 0x005106f6 (ecx = index,
// esi = the record address on return), stores that address at 0x00a6987c and popal restores every
// register. popal leaves the flags alone, so the flags are those 0x005106f6 left behind.
// FUNCTION: GP4 0x005107c6
void __cdecl FUN_005107c6(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = GP4_GLOBAL(uint32_t, 0x00a69878);
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x005106f6, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005107c6, FUN_005107c6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
