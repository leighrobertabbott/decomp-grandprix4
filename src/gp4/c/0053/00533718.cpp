#include <gp4/gp4.h>

// `call 0x0052a30f ; ret`: tail wrapper. The callee refreshes a front-end screen: when [0x007aafd3]
// is -1 it clears [0x007aafdf], sets [0x007aafe0] = 0xff and runs 0x0054b0ea / 0x0052a36d (a wait
// on 0x0053c8af); the poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x00533718
void __cdecl FUN_00533718(gp4::Regs* r) {
    gp4::call_regs(0x0052a30f, *r);
}
GP4_IMPL(0x00533718, FUN_00533718, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
