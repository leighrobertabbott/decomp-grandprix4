#include <gp4/gp4.h>

// `call 0x0052fa3d ; ret`: tail wrapper. The callee refreshes a front-end screen (stores the result
// of 0x00530774, picks 0x0052faa1 or 0x0052fbca by [0x007aafd3], both of which wait through
// 0x0053c8af) and so reaches the message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and
// PeekMessageA. Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x00532ec8
void __cdecl FUN_00532ec8(gp4::Regs* r) {
    gp4::call_regs(0x0052fa3d, *r);
}
GP4_IMPL(0x00532ec8, FUN_00532ec8, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
