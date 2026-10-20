#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the eight-dword table at 0x00828214 (0x0053414a), then runs 0x00534123 with eax = 3 and esi = 0x17c
// (it stores the 0x0054d1ad value for index 0x17c into entry 3 and walks the node list at 0x00852837), and
// finally sets the (unaligned) dword 0x00828293 to 0xb8. 0x00534123 is a pushal/popal routine, so eax stays 3 and
// esi stays 0x17c; ecx and edi come from 0x0053414a and the flags from the list walk.
// FUNCTION: GP4 0x00534db5
void __cdecl FUN_00534db5(gp4::Regs* r) {
    gp4::call_regs(0x0053414a, *r);
    r->eax = 3u;
    r->esi = 0x17cu;
    gp4::Regs c = *r;
    gp4::call_regs(0x00534123, c);
    GP4_GLOBAL(uint32_t, 0x00828293) = 0xb8u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00534db5, FUN_00534db5, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=head_00852837+tab_00835846")
