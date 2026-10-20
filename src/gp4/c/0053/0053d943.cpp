#include <gp4/gp4.h>
#include <gp4/flags.h>

// `mov eax, 0x200 ; call 0x0053d964 ; ret`: toggles menu option bit 0x200 (see 0x0053d964).
// FUNCTION: GP4 0x0053d943
void __cdecl FUN_0053d943(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0x200u;
    gp4::call_regs(0x0053d964, c);
    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053d943, FUN_0053d943, "regs(ecx:u32, edx:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+items_0053dc57+mask_00827fd4")
