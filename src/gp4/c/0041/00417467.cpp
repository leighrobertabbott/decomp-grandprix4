#include <gp4/gp4.h>

// FUNCTION: GP4 0x00417467
// Tail wrapper: `call 0x00417415 ; ret` (runs 0x0041724d on the first two 0x300-byte records at
// 0x006380d8, then 0x004171b6 which reads esi and eax, and leaves ebx = 0x006380d8).
void __cdecl FUN_00417467(gp4::Regs* r) {
    gp4::call_regs(0x00417415, *r);
}
GP4_IMPL(0x00417467, FUN_00417467, "regs(esi:ptr[0xed]:bytes, eax:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
