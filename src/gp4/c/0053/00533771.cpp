#include <gp4/gp4.h>

// `call 0x0053ed83 ; ret`: tail wrapper. The callee asks 0x00509ca0 and, for the results 0xb and 9,
// waits through 0x0053c8c8 (script 0x321); that poll reaches the message pump 0x00509b50 (via
// 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as the callee leaves
// them.
// FUNCTION: GP4 0x00533771
void __cdecl FUN_00533771(gp4::Regs* r) {
    gp4::call_regs(0x0053ed83, *r);
}
GP4_IMPL(0x00533771, FUN_00533771, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
