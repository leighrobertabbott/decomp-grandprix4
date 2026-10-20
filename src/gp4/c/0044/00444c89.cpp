#include <gp4/gp4.h>

// Opcode 3 record writer: runs 0x004450e5 with al = 3, then raises the byte flag 0x0095404e. pushal / popal
// restore every register; only the flags left by the callee survive (the byte store changes none).
// FUNCTION: GP4 0x00444c89
void __cdecl FUN_00444c89(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | 3u;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x004450e5, c);
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00444c89, FUN_00444c89, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041104f+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+rec_004450e5")
