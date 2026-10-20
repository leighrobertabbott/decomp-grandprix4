#include <gp4/gp4.h>
#include <gp4/flags.h>

// Pops key-queue words with 0x0052d729 until it reports (carry set) that the high byte of the popped
// word is zero, i.e. the queue is drained or a plain character came out. eax and the flags are those
// of the last call.
// FUNCTION: GP4 0x0052d752
void __cdecl FUN_0052d752(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    do {
        gp4::call_regs(0x0052d729, c);
    } while ((c.eflags & gp4::flags::CF) == 0u);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052d752, FUN_0052d752, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0")
