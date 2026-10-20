#include <gp4/gp4.h>

// `mov byte [0x007ab020], 3` and then falls through into 0x005400fa (the dialog sequence: 0x0054036e,
// the wait loop 0x0052cb10 with a count of 4, 0x0054036e, [0x007ab020] = 0, 0x0054fbfb, 0x0053c8c8 on
// the item 0x00830634, [0x007ab020] = 3, 0x0053f6d6). The wait loop reaches the message pump 0x00509b50
// and so PeekMessageA. Registers and flags come straight back from the fall-through target.
// FUNCTION: GP4 0x005400f3
void __cdecl FUN_005400f3(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007ab020) = 3u;
    gp4::call_regs(0x005400fa, *r);
}
GP4_IMPL(0x005400f3, FUN_005400f3, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
