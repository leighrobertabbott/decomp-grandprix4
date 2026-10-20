#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper over the item-chain flag walk 0x0053e4db: esi = the item chain header 0x00848c6e and
// eax = 1 when the four dwords at 0x00b1b8c8 are all zero, else 0 (the bits shifted into the item
// flags). popal restores every register; the flags are those of the callee's last flag writer.
// FUNCTION: GP4 0x00540be6
void __cdecl FUN_00540be6(gp4::Regs* r) {
    const uint32_t* cells = GP4_ARRAY(uint32_t, 0x00b1b8c8);
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0u;
    const uint32_t any = cells[0] | cells[1] | cells[2] | cells[3];
    c.ebx = any;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(any));
    if (any == 0u) {
        c.eax = 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(1u));
    }
    c.esi = 0x00848c6eu;
    gp4::call_regs(0x0053e4db, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00540be6, FUN_00540be6, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=cells_00540be6")
