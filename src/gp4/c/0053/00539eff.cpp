#include <gp4/gp4.h>

// `call 0x00539f40 ; call 0x00531b2c ; ret`: loads the 4-word record at esi sign-extended into
// ebx/eax/ebp/ecx and advances esi by 8 (0x00539f40), then runs the empty routine 0x00531b2c.
// Registers and flags come back from the first call.
// FUNCTION: GP4 0x00539eff
void __cdecl FUN_00539eff(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.st_xin_mask = 0;
    state.st_xout_mask = 0;
    gp4::call_regs(0x00539f40, state);
    gp4::cdecl_<0x00531b2c, void>();

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->ebp = state.ebp;
    r->esi = state.esi;
    r->eflags = state.eflags;
}
GP4_IMPL(0x00539eff, FUN_00539eff, "regs(esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, ebp, esi, cf, pf, af, zf, sf, of)")
