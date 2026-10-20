#include <gp4/gp4.h>

// `call 0x0054fdfd ; call 0x00549d81 ; ret`: the registers each call leaves carry into the next; registers and
// flags come back from the second call.
// FUNCTION: GP4 0x0054fcb5
void __cdecl FUN_0054fcb5(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.st_xin_mask = 0;
    state.st_xout_mask = 0;
    gp4::call_regs(0x0054fdfd, state);
    gp4::call_regs(0x00549d81, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x0054fcb5, FUN_0054fcb5, "regs(esi:ptr[0x10]:layout=box_0054fe11) -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")
