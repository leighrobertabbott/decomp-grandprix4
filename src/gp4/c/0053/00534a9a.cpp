#include <gp4/gp4.h>

// Tail wrapper: runs 0x0053414a (fills the eight-dword table at 0x00828214) and returns its registers and flags.
// FUNCTION: GP4 0x00534a9a
void __cdecl FUN_00534a9a(gp4::Regs* r) {
    gp4::call_regs(0x0053414a, *r);
}
GP4_IMPL(0x00534a9a, FUN_00534a9a, "regs() -> (ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_00835846")
