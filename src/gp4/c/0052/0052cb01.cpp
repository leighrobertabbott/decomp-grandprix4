#include <gp4/gp4.h>

// `push ebp ; mov ebp, esp ; mov eax, [ebp+8] ; call 0x0052cb37 ; leave ; ret 4`: stdcall wrapper
// that loads its argument into eax and runs the wait loop 0x0052cb37 (a pushal/popal routine that
// polls the message pump, so the cases end in PeekMessageA). The loop restores every register, so
// eax comes back as the argument.
// FUNCTION: GP4 0x0052cb01
int __stdcall FUN_0052cb01(int arg) {
    gp4::Regs c = {};
    c.eax = static_cast<uint32_t>(arg);
    gp4::call_regs(0x0052cb37, c);
    return static_cast<int>(c.eax);
}
GP4_IMPL(0x0052cb01, FUN_0052cb01, "stdcall(i32) -> i32; globals=fuzz:i32; emu=skip:win32 PeekMessageA")
