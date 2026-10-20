#include <gp4/gp4.h>

// `call 0x00539f40 ; mov edx, 0xa7 ; call 0x00531b36 ; ret`: loads the 4-word record at esi
// sign-extended into ebx/eax/ebp/ecx and advances esi by 8 (0x00539f40), sets edx = 0xa7, then runs
// the empty routine 0x00531b36. Flags come back from the first call.
// FUNCTION: GP4 0x00539eda
void __cdecl FUN_00539eda(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.st_xin_mask = 0;
    state.st_xout_mask = 0;
    gp4::call_regs(0x00539f40, state);
    gp4::cdecl_<0x00531b36, void>();

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = 0xa7u;
    r->ebp = state.ebp;
    r->esi = state.esi;
    r->eflags = state.eflags;
}
GP4_IMPL(0x00539eda, FUN_00539eda, "regs(esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, ebp, esi, cf, pf, af, zf, sf, of)")
