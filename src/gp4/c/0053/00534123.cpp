#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: esi = table[esi] (0x0054d1ad), stores it into entry eax of the dword table at
// 0x00828214 (0x0053413f), then walks the node list that starts at the cell 0x00852837 with
// eax = 0x8000 (0x0053e612, which sets flag bit 6 of the nodes reached by the bits of eax). Every GPR
// is restored; the flags are those the list walk ends on (0 - 0).
// FUNCTION: GP4 0x00534123
void __cdecl FUN_00534123(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054d1ad, c);
    gp4::call_regs(0x0053413f, c);
    c.eax = 0x8000u;
    c.esi = 0x00852837u;
    gp4::call_regs(0x0053e612, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00534123, FUN_00534123, "regs(eax:u32[0..15], esi:u32[0..0x3ff]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=head_00852837+tab_00835846")
