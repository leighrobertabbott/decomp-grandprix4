#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs the side-line distance pass 0x005558c6 over every record of the 0x160-byte track
// ring at 0x008943a0, in two do-while walks that end only on equality: records 0.. up to and including
// the lap end [0x00866ecc] (the end record gets one more call after the loop), then from the pit start
// [0x00891500] up to the pit end [0x00866ef8] (again one more call for the end record). popal restores
// every GPR; the flags are those the last call leaves.
// FUNCTION: GP4 0x00555903
void __cdecl FUN_00555903(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.edi = 0x008943a0u;
    do {
        gp4::call_regs(0x005558c6, c);
        c.edi += 0x160u;
    } while (c.edi != GP4_GLOBAL(uint32_t, 0x00866ecc));
    gp4::call_regs(0x005558c6, c);

    c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
    do {
        gp4::call_regs(0x005558c6, c);
        c.edi += 0x160u;
    } while (c.edi != GP4_GLOBAL(uint32_t, 0x00866ef8));
    gp4::call_regs(0x005558c6, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00555903, FUN_00555903, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:f64+layout=multiply_0040ce7b+ring_00555b28+geo_005558c6")
