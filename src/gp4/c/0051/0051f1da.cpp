#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs 0x00522e54 (which picks a ring-record pointer in edi) with ebx, ecx, edx, ebp, esi and edi saved
// around it, then returns the edi it computed in eax. The flags are those the callee leaves.
// FUNCTION: GP4 0x0051f1da
void __cdecl FUN_0051f1da(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00522e54, c);
    r->eax = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f1da, FUN_0051f1da, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023")
