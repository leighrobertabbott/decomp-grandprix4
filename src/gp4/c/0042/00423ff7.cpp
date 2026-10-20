#include <gp4/gp4.h>

// One simulation sub-step: five update routines run back to back (0x00423c93, 0x0040233e,
// 0x00423ea1, 0x00402bb9, 0x00423de4). The registers each leaves behind carry into the next.
// FUNCTION: GP4 0x00423ff7
void __cdecl FUN_00423ff7(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x00423c93, state);
    gp4::call_regs(0x0040233e, state);
    gp4::call_regs(0x00423ea1, state);
    gp4::call_regs(0x00402bb9, state);
    gp4::call_regs(0x00423de4, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x00423ff7, FUN_00423ff7, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
