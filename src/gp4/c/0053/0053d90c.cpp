#include <gp4/gp4.h>
#include <gp4/flags.h>

// `mov eax, 1 ; call 0x0053d964 ; ret`: toggles menu option bit 1 (see 0x0053d964).
// FUNCTION: GP4 0x0053d90c
void __cdecl FUN_0053d90c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 1u;
    gp4::call_regs(0x0053d964, c);
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053d90c, FUN_0053d90c, "regs(ecx:u32, edx:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+items_0053dc57+mask_00827fd4")
