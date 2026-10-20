#include <gp4/gp4.h>

// `call 0x0054ad8a ; call 0x0054a2c3 ; ret`: copies the 22-byte per-car flag table and latches code
// 0x10 (0x0054ad8a), then runs the menu/dialog wait state machine 0x0054a2c3. The registers and
// flags each call leaves carry into the next; everything comes back from the second call.
// FUNCTION: GP4 0x0054ad7f
void __cdecl FUN_0054ad7f(gp4::Regs* r) {
    gp4::call_regs(0x0054ad8a, *r);
    gp4::call_regs(0x0054a2c3, *r);
}
GP4_IMPL(0x0054ad7f, FUN_0054ad7f, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043bcd4; emu=skip:win32 PeekMessageA")
