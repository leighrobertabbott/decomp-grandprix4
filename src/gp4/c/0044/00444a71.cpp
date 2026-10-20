#include <gp4/gp4.h>

// pushal wrapper: runs 0x00444917 for the 23 values ecx = 0 .. 22 (it averages the records selected by bit ecx of
// every entry of the table [0x0062a048]). popal restores every register; the flags are those of the final
// `cmp ecx, 0x17` (equal: ZF | PF).
// FUNCTION: GP4 0x00444a71
void __cdecl FUN_00444a71(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    for (uint32_t i = 0; i != 0x17u; ++i) {
        call.ecx = i;
        gp4::call_regs(0x00444917, call);
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00444a71, FUN_00444a71, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0062a048+state_00444a71")
