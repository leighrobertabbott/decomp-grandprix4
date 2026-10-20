#include <gp4/gp4.h>

// Tail wrapper: `call 0x0054a25d ; ret`. The callee runs 0x0054aec8 (resets [0x007aafd3]/[0x007aafd8],
// then waits through 0x0053c8af / 0x0053c8c8, which poll 0x0053d350; that poll reaches the message
// pump 0x00509b50 via 0x0054a2c3 and so PeekMessageA) and then 0x005ae990. Registers and flags come
// straight back from it.
// FUNCTION: GP4 0x005357c7
void __cdecl FUN_005357c7(gp4::Regs* r) {
    gp4::call_regs(0x0054a25d, *r);
}
GP4_IMPL(0x005357c7, FUN_005357c7, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
