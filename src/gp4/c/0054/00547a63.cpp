#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the table index of the byte [0x00822278] + 1 (0x005484eb), places eax = [0x00831ad4] + 0x50,
// ebx = [0x00831ad0] and edx = index + 0x1a6 (a frame number) and runs the pushal/popal frame
// registrar 0x0052805d: it restores every register, so only eax/ebx/edx as loaded here and the callee's
// flags are left.
// FUNCTION: GP4 0x00547a63
void __cdecl FUN_00547a63(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00822278);
    gp4::call_regs(0x005484eb, c);
    c.edx = c.eax + 0x1a6u;
    c.ebx = GP4_GLOBAL(uint32_t, 0x00831ad0);
    c.eax = GP4_GLOBAL(uint32_t, 0x00831ad4) + 0x50u;
    gp4::call_regs(0x0052805d, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00547a63, FUN_00547a63, "regs(esi:ptr[0x10]:bytes, edi:ptr[8]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_005388ed+cells_00527eae+slot8_005388ba")
