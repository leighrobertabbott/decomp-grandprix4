#include <gp4/gp4.h>

// `call 0x00537750 ; ret`: tail wrapper. The callee is a pushal/popal routine that formats
// [0x007c2e7c] as hours:minutes:seconds into 0x00823100 (script 0xb77 / 0xb78) and then waits
// through 0x0053c8c8; that poll reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3)
// and so PeekMessageA. Registers come back restored; the flags are the callee's.
// FUNCTION: GP4 0x0053376b
void __cdecl FUN_0053376b(gp4::Regs* r) {
    gp4::call_regs(0x00537750, *r);
}
GP4_IMPL(0x0053376b, FUN_0053376b, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
