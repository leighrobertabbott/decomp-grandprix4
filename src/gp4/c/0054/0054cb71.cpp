#include <gp4/gp4.h>

// push eax; push esi; push edi; pop edi; pop esi; pop eax; ret
// Saves and restores three registers: no effect on registers, flags or memory.
// FUNCTION: GP4 0x0054cb71
void __cdecl FUN_0054cb71(gp4::Regs* r) {
    (void)r;
}
GP4_IMPL(0x0054cb71, FUN_0054cb71, "regs() -> ()")
