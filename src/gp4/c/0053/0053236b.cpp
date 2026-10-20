#include <gp4/gp4.h>

// Tail wrapper: `call 0x005476b2 ; ret`. The callee runs the front-end screen loop (it polls 0x0053d350
// and dispatches on the key action); the poll reaches the message pump 0x00509b50 (via 0x0054a2c3) and
// so PeekMessageA. Registers and flags come straight back from it.
// FUNCTION: GP4 0x0053236b
void __cdecl FUN_0053236b(gp4::Regs* r) {
    gp4::call_regs(0x005476b2, *r);
}
GP4_IMPL(0x0053236b, FUN_0053236b, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
