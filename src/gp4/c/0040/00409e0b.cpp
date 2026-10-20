#include <gp4/gp4.h>

// Runs the 16-bit-register 64/32 divider 0x409e29 in signed mode: the mode word DAT_00623e14 is
// set to 0x8000 (negative) first. All registers are whatever the divider leaves. The divider's
// flags come from its closing "add esp, 0x20" (a function of the stack pointer, not of the data),
// so as in its own reconstruction they are modelled as cleared rather than copied from the
// bridged call, whose stack depth differs from the original's.
// FUNCTION: GP4 0x00409e0b
void __cdecl FUN_00409e0b(gp4::Regs* r) {
    GP4_GLOBAL(uint16_t, 0x00623e14) = 0x8000;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00409e29, callee);

    r->eax = callee.eax;
    r->ebx = callee.ebx;
    r->ecx = callee.ecx;
    r->edx = callee.edx;
    r->esi = callee.esi;
    r->edi = callee.edi;
    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x00409e0b, FUN_00409e0b, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax,ebx,ecx,edx,esi,edi,cf,pf,af,zf,sf,of); globals=fuzz:i32")
