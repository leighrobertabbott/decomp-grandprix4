#include <gp4/gp4.h>

// Menu screen wrapper: 0x0054acd3 reports through the carry flag whether the screen should run (it
// enters the key-wait loop in network mode and otherwise sets carry at once); then the screen event loop
// 0x005330b0 runs and 0x0054ad3c closes the wait. Registers and flags come back from the last call that
// ran.
// FUNCTION: GP4 0x0053309c
void __cdecl FUN_0053309c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054acd3, c);
    if ((c.eflags & 1u) != 0u) {
        gp4::call_regs(0x005330b0, c);
        gp4::call_regs(0x0054ad3c, c);
    }
    *r = c;
}
GP4_IMPL(0x0053309c, FUN_0053309c, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3; emu=skip:win32 PeekMessageA")
