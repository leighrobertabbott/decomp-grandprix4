#include <gp4/gp4.h>

// Stores 0x4210 through 0x00531af4, loads the 4-word record at esi sign-extended into ebx/eax/ebp/ecx
// (advancing esi by 8) through 0x00539f40, then runs the empty routine 0x00531b2b. Everything the
// caller sees (ebx, eax, ebp, ecx, esi and the flags) comes from the record loader.
// FUNCTION: GP4 0x00539eea
void __cdecl FUN_00539eea(gp4::Regs* r) {
    gp4::Regs store = *r;
    store.st_in = 0;
    store.st_out = 0;
    store.st_xin_mask = 0;
    store.st_xout_mask = 0;
    store.eax = 0x4210u;
    gp4::call_regs(0x00531af4, store);

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.st_xin_mask = 0;
    state.st_xout_mask = 0;
    gp4::call_regs(0x00539f40, state);
    gp4::cdecl_<0x00531b2b, void>();

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->ebp = state.ebp;
    r->esi = state.esi;
    r->eflags = state.eflags;
}
GP4_IMPL(0x00539eea, FUN_00539eea, "regs(esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
