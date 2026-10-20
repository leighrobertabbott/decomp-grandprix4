#include <gp4/gp4.h>

// `call 0x005411b3 ; ret`: tail wrapper. The callee collects the selected cars of 0x007c2a48 into
// 0x0081d320 and runs a front-end screen loop (waits on 0x0053c8af and dispatches through a jump
// table); the poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x00533724
void __cdecl FUN_00533724(gp4::Regs* r) {
    gp4::call_regs(0x005411b3, *r);
}
GP4_IMPL(0x00533724, FUN_00533724, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
