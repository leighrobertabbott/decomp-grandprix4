#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores edi + ecx at [0x6975bc] (edi is saved and restored around the add), then runs the command
// dispatcher 0x00531f9c with ax = 0x50 (the upper half of eax is the caller's). eax and the flags are the
// dispatcher's (the handler's); every other register comes back unchanged.
// FUNCTION: GP4 0x00510fad
void __cdecl FUN_00510fad(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006975bc) = r->edi + r->ecx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffff0000u) | 0x50u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->edi, r->ecx));
    gp4::call_regs(0x00531f9c, c);

    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00510fad, FUN_00510fad, "regs(eax:u32, ecx:u32, edi:u32) -> (eax, cf, pf, af, zf, sf, of)")
