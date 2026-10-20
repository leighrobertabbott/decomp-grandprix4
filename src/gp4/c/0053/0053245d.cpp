#include <gp4/gp4.h>

// `call 0x0052bd40 ; ret`: tail wrapper. The callee does nothing when [0x7aafd3] is 0xff and otherwise
// runs the dialog routine 0x0052bd4f, which waits on the menu runner 0x0053c8c8 (-> 0x0053d350 ->
// 0x0054a2c3 -> the message pump 0x00509b50, hence PeekMessageA); registers and flags come back as
// the callee leaves them.
// FUNCTION: GP4 0x0053245d
void __cdecl FUN_0053245d(gp4::Regs* r) {
    gp4::call_regs(0x0052bd40, *r);
}
GP4_IMPL(0x0053245d, FUN_0053245d, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
