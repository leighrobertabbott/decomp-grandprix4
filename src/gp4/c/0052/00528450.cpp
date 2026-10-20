#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frame-file closer. 0x00527d03 (`stc; ret`) runs first and its carry skips the `jb` loop that would
// load every frame number from [edi+0xc] to [edi+0x10] through 0x00527dee, then 0x0053646a closes the
// frame file. The loop is kept as the original wrote it. Every register comes back from 0x0053646a
// unchanged; the flags are those of its `cmp byte [0x81ea2c], 0`.
// FUNCTION: GP4 0x00528450
void __cdecl FUN_00528450(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00527d03, c);
    if ((c.eflags & 1u) == 0u) {
        c.eax = GP4_FIELD(uint32_t, c.edi, 0xc);
        c.ecx = GP4_FIELD(uint32_t, c.edi, 0x10) - c.eax + 1u;
        do {
            gp4::call_regs(0x00527dee, c);
            c.eax = c.eax + 1u;
            c.ecx = c.ecx - 1u;
        } while (c.ecx != 0u);
    }
    gp4::call_regs(0x0053646a, c);
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00528450, FUN_00528450, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
