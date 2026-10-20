#include <gp4/gp4.h>

// `call 0x00445342 ; call 0x004454d2 ; ret`: runs the stream routine 0x00445342, then clears bit 7 of byte +0x78 of
// the car records [0x0066d820] / [0x0066d828] with 0x004454d2. The registers each call leaves carry into the next;
// registers and flags come back from the second call.
// FUNCTION: GP4 0x00445303
void __cdecl FUN_00445303(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x00445342, state);
    gp4::call_regs(0x004454d2, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x00445303, FUN_00445303, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0040fa01")
