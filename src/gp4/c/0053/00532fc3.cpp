#include <gp4/gp4.h>

// `call 0x00528b34 ; ret`: tail wrapper. The callee is the main menu event loop of a front-end
// screen (it polls 0x0053c8af and dispatches through a jump table); the poll reaches the message
// pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come back as
// the callee leaves them.
// FUNCTION: GP4 0x00532fc3
void __cdecl FUN_00532fc3(gp4::Regs* r) {
    gp4::call_regs(0x00528b34, *r);
}
GP4_IMPL(0x00532fc3, FUN_00532fc3, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
