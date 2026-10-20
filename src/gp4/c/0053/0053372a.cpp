#include <gp4/gp4.h>

// `call 0x00540035 ; ret`: tail wrapper. The callee sends command 3 (al = 3, ebx = [0x00ae6b64],
// edx = 0) through 0x005403ca, sets [0x007ab020] = 4 and runs 0x0053f6d6, which can wait through
// 0x0053c8c8; that poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x0053372a
void __cdecl FUN_0053372a(gp4::Regs* r) {
    gp4::call_regs(0x00540035, *r);
}
GP4_IMPL(0x0053372a, FUN_0053372a, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
