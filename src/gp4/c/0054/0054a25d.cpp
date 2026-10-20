#include <gp4/gp4.h>

// `call 0x0054aec8 ; call 0x005ae990 ; ret`: runs the menu-screen setup/loop 0x0054aec8 (clears the
// flags at 0x7aafd3/0x7aafd8, draws and runs menu 0x84be42 or 0x8331fc) then a bare `ret` routine.
// The registers and flags the first call leaves come straight back.
// FUNCTION: GP4 0x0054a25d
void __cdecl FUN_0054a25d(gp4::Regs* r) {
    gp4::call_regs(0x0054aec8, *r);
    gp4::call_regs(0x005ae990, *r);
}
GP4_IMPL(0x0054a25d, FUN_0054a25d, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
