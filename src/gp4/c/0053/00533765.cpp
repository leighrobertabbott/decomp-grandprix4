#include <gp4/gp4.h>

// `call 0x0053080f ; ret`: tail wrapper. The callee runs 0x0053090c, loads the "gconfig.txt" record
// (0x00530931) and refreshes the screen with 0x0053081f, which can wait through 0x0053c8c8; that
// poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA.
// Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x00533765
void __cdecl FUN_00533765(gp4::Regs* r) {
    gp4::call_regs(0x0053080f, *r);
}
GP4_IMPL(0x00533765, FUN_00533765, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
