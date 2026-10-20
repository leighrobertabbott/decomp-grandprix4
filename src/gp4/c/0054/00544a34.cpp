#include <gp4/gp4.h>

// Loads eax = 0x1ae8 and runs the running-total advance routine 0x00538a24 (which adds eax to the
// dword 0x0082fcb0 and returns CF set when the addition carried). Registers and flags are whatever
// that routine leaves.
// FUNCTION: GP4 0x00544a34
void __cdecl FUN_00544a34(gp4::Regs* r) {
    r->eax = 0x1ae8u;
    gp4::call_regs(0x00538a24, *r);
}
GP4_IMPL(0x00544a34, FUN_00544a34, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=pool_00538a24+total_00544a34")
