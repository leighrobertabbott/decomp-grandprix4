#include <gp4/gp4.h>

// Tail wrapper: `call 0x0054f0bb ; ret`. The callee shifts 0x80 left by the circuit number [0x007ad894],
// arms the node at 0x008429b0 (byte +0xd = 1, dword +0x14 = 0x614) and runs the selection loop
// 0x0054f1a1 between two calls of 0x0053e4db; the loop polls 0x0053d350, which reaches the message pump
// 0x00509b50 (via 0x0054a2c3) and so PeekMessageA. Registers and flags come straight back from it.
// FUNCTION: GP4 0x0053235a
void __cdecl FUN_0053235a(gp4::Regs* r) {
    gp4::call_regs(0x0054f0bb, *r);
}
GP4_IMPL(0x0053235a, FUN_0053235a, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
