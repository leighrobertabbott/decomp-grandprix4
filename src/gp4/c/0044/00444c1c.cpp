#include <gp4/gp4.h>

// Opcode 1 record writer: runs 0x004450e5 with al = 1. pushal / popal restore every register; only the flags
// left by the callee survive.
// FUNCTION: GP4 0x00444c1c
void __cdecl FUN_00444c1c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | 1u;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x004450e5, c);
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00444c1c, FUN_00444c1c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041104f+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+rec_004450e5")
