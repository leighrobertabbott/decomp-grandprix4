#include <gp4/gp4.h>

// FUNCTION: GP4 0x00416b4c
// Tail wrapper: `call 0x00416add ; ret` (runs 0x004169c3 on the first two 0x300-byte records
// at 0x006380d8, then stores -((rec0.+0x30 + rec1.+0x30) >> 1) at 0x00601ab4).
void __cdecl FUN_00416b4c(gp4::Regs* r) {
    gp4::call_regs(0x00416add, *r);
}
GP4_IMPL(0x00416b4c, FUN_00416b4c, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
