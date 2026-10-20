#include <gp4/gp4.h>

// FUNCTION: GP4 0x00416b52
// Tail wrapper: `call 0x00416b13 ; ret` (runs 0x004169c3 on the third and fourth 0x300-byte
// records at 0x006380d8, then stores -((rec2.+0x30 + rec3.+0x30) >> 1) at 0x00601ab0).
void __cdecl FUN_00416b52(gp4::Regs* r) {
    gp4::call_regs(0x00416b13, *r);
}
GP4_IMPL(0x00416b52, FUN_00416b52, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
