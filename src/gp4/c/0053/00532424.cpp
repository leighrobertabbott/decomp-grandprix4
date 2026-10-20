#include <gp4/gp4.h>

// Clears the byte 0x008293fc, then tail-calls 0x00535afe (a register routine that runs 0x00503530 under
// pushal/popal and then, by [0x007aafd3] == 0xff, 0x00535b1d or 0x00535b9a). Registers and flags come
// back as the callee leaves them.
// FUNCTION: GP4 0x00532424
void __cdecl FUN_00532424(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x008293fc) = 0u;
    gp4::call_regs(0x00535afe, *r);
}
GP4_IMPL(0x00532424, FUN_00532424, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3+list_00549db9+pool_00549db9+surfcopy_00549d55+case_0054fd4d+ext_00536031 ; emu=skip:win32 PeekMessageA")
