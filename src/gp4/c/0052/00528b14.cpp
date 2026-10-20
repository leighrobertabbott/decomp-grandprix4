#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper over the item-chain flag walk 0x0053e4db: esi = the item chain header 0x0084eebe and
// eax = 0 when the byte [0x0081d22b] is nonzero, else 3 (the bits shifted into the item flags). popal
// restores every register; the flags are those of the callee's last flag writer.
// FUNCTION: GP4 0x00528b14
void __cdecl FUN_00528b14(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x0084eebeu;
    c.eax = 0u;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0081d22b);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(mode, 0u));
    if (mode == 0u) {
        c.eax = 3u;
    }
    gp4::call_regs(0x0053e4db, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00528b14, FUN_00528b14, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=cnt_0081d22b")
