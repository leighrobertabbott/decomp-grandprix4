#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: the menu selection step 0x0053cb41, then 0x00540131. popal restores every
// register but not the flags, which are those 0x00540131 leaves.
// FUNCTION: GP4 0x00540aff
void __cdecl FUN_00540aff(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0053cb41, call);
    gp4::call_regs(0x00540131, call);
    r->eflags = gp4::flags::merge(r->eflags, call.eflags);
}
GP4_IMPL(0x00540aff, FUN_00540aff, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32) -> (cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053d8c5+flags_0084869c+slots_00541d5a+kind_00541cdb+host_00b1bb40")
