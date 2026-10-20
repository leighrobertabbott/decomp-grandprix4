#include <gp4/gp4.h>

// Frame item wrapper: the record at esi is queued by 0x0054e8e7 with edx = 0xd0 as the base value.
// Every register and the flags come back from the callee.
// FUNCTION: GP4 0x0054e814
void __cdecl FUN_0054e814(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edx = 0xd0u;
    gp4::call_regs(0x0054e8e7, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054e814, FUN_0054e814, "regs(esi:ptr[0x40]:layout=item_0054e8e7, eax:u32, ebx:u32, ecx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=list_00549db9+item_0054e8e7")
