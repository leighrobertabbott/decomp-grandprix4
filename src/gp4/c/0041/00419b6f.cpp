#include <gp4/gp4.h>

// Runs the four per-slot update routines in order. Each one is a register-convention routine that
// takes no inputs and leaves its results (eax/ebx/edx/ebp/flags) in the registers, so one register
// block is threaded through all four calls and handed back as the wrapper's outputs.
// FUNCTION: GP4 0x00419b6f
void __cdecl FUN_00419b6f(gp4::Regs* r) {
    gp4::Regs regs = *r;
    gp4::call_regs(0x00419d1c, regs);
    gp4::call_regs(0x00419d76, regs);
    gp4::call_regs(0x00419e97, regs);
    gp4::call_regs(0x00419afd, regs);

    r->eax = regs.eax;
    r->ebx = regs.ebx;
    r->ecx = regs.ecx;
    r->edx = regs.edx;
    r->esi = regs.esi;
    r->edi = regs.edi;
    r->ebp = regs.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (regs.eflags & 0x8d5u);
}
GP4_IMPL(0x00419b6f, FUN_00419b6f, "regs() -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00419afd")
