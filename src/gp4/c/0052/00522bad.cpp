#include <gp4/gp4.h>
#include <gp4/flags.h>

// `call 0x00522bb8 ; call 0x00522c00 ; ret`: the first call copies the car-code list (al = last byte
// copied), the second resets the three 22-entry dword tables to 0x20000000 (ebx = 0x58, ecx = 0).
// Registers and flags of the first call carry into the second; everything comes back from the second.
// FUNCTION: GP4 0x00522bad
void __cdecl FUN_00522bad(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.st_xin_mask = 0;
    state.st_xout_mask = 0;
    gp4::call_regs(0x00522bb8, state);
    gp4::call_regs(0x00522c00, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = gp4::flags::merge(r->eflags, state.eflags);
}
GP4_IMPL(0x00522bad, FUN_00522bad, "regs(eax:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00443278")
