#include <gp4/gp4.h>

// Tail wrapper: `call 0x0052ab5a ; ret`. The callee builds a text line at 0x00823200 from the script
// ids (0x2bb..0x2cd, 0xb90 + ([0x007a943c] & 0x3f)) and the words 0x007ad89e/a0/a2, then waits for a key
// through 0x0053c8af, which polls 0x0053d350; that poll reaches the message pump 0x00509b50 (via
// 0x0054a2c3) and so PeekMessageA. Registers and flags come straight back from it.
// FUNCTION: GP4 0x00532373
void __cdecl FUN_00532373(gp4::Regs* r) {
    gp4::call_regs(0x0052ab5a, *r);
}
GP4_IMPL(0x00532373, FUN_00532373, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
