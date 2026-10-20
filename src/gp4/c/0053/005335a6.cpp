#include <gp4/gp4.h>

// `call 0x00546a8c ; ret`: tail wrapper. The callee builds the screen state (0x00548e66, 0x00548e0b,
// 0x00548cec), sets [0x00849781] = 1 and loops on 0x0053c8af, dispatching through a three-entry
// jump table; the poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x005335a6
void __cdecl FUN_005335a6(gp4::Regs* r) {
    gp4::call_regs(0x00546a8c, *r);
}
GP4_IMPL(0x005335a6, FUN_005335a6, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
