#include <gp4/gp4.h>

// `call 0x005489d7 ; ret`: tail wrapper. The callee builds the list of the front-end screen (script
// 0x2fa, the 22 entries of 0x004ff220) and then waits through 0x0053c8af, which polls 0x0053d350; that
// poll reaches the message pump 0x00509b50 (via 0x0054a2c3) and so PeekMessageA. Registers and flags
// come back as the callee leaves them.
// FUNCTION: GP4 0x0053295b
void __cdecl FUN_0053295b(gp4::Regs* r) {
    gp4::call_regs(0x005489d7, *r);
}
GP4_IMPL(0x0053295b, FUN_0053295b, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
