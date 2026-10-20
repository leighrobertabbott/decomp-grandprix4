#include <gp4/gp4.h>

// Tail wrapper: `call 0x00418afe ; ret`. Registers and flags come straight back from the callee.
// FUNCTION: GP4 0x00418af8
void __cdecl FUN_00418af8(gp4::Regs* r) {
    gp4::call_regs(0x00418afe, *r);
}
GP4_IMPL(0x00418af8, FUN_00418af8, "regs(esi:ptr[0x260]:layout=rec_00418c68) -> (cf, pf, af, zf, sf, of); globals=layout=state_00418c68")
