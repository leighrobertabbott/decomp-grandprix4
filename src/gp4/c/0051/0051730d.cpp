#include <gp4/gp4.h>

// `call 0x0051fcdb ; call 0x00517318 ; ret`: converts the low byte of eax to packed BCD
// (0x0051fcdb), then writes it as two display characters at [edi] (0x00517318, edi += 2).
// The registers each call leaves carry into the next; registers and flags come back from the
// second call.
// FUNCTION: GP4 0x0051730d
void __cdecl FUN_0051730d(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x0051fcdb, state);
    gp4::call_regs(0x00517318, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x0051730d, FUN_0051730d, "regs(eax:u32, edi:ptr[0x10]:bytes) -> (eax, edi, cf, pf, af, zf, sf, of)")
