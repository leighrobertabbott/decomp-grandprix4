#include <gp4/gp4.h>

// `pushal ; pushf ; push eax ; mov eax, 0x13 ; call 0x0052cb10 ; pop eax ; popf ; popal ; ret`:
// runs the wait loop of 0x0052cb10 with a counter of 0x13 and gives every register and the flags
// back as they were on entry.
// FUNCTION: GP4 0x0052cad8
void __cdecl FUN_0052cad8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0x13u;
    gp4::call_regs(0x0052cb10, c);
}
GP4_IMPL(0x0052cad8, FUN_0052cad8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
