#include <gp4/gp4.h>

// `call 0x0051fcdb ; call 0x00510e7c ; ret`: converts the low byte of eax to packed BCD (0x0051fcdb), then
// writes it as two display characters at [edi] (0x00510e7c). The registers and flags each call leaves carry
// into the next; everything comes back from the second call.
// FUNCTION: GP4 0x00510e71
void __cdecl FUN_00510e71(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.st_xin_mask = 0;
    state.st_xout_mask = 0;
    gp4::call_regs(0x0051fcdb, state);
    gp4::call_regs(0x00510e7c, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x00510e71, FUN_00510e71, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187")
