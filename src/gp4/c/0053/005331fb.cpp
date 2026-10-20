#include <gp4/gp4.h>

// `call 0x0054f0fc ; ret`: tail wrapper. The callee sets [0x008429bd] = 1, runs 0x0053e4db, sets
// [0x008429c4] = 0x6fc and enters the front-end screen loop 0x0054f1a1, which waits through
// 0x0053d350 / 0x0053c8af; the poll reaches the message pump 0x00509b50 (via 0x0054a2c3) and so
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x005331fb
void __cdecl FUN_005331fb(gp4::Regs* r) {
    gp4::call_regs(0x0054f0fc, *r);
}
GP4_IMPL(0x005331fb, FUN_005331fb, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
