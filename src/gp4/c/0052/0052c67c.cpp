#include <gp4/gp4.h>

// `push edx ; call 0x00538f91 ; pop edx ; mov edx, 0 ; ret`: runs the menu value handler with the
// registers as they came in; eax and the flags are the handler's, edx is cleared (the handler
// already clears it, the pop only restores it for the final `mov`).
// FUNCTION: GP4 0x0052c67c
void __cdecl FUN_0052c67c(gp4::Regs* r) {
    gp4::call_regs(0x00538f91, *r);
    r->edx = 0u;
}
GP4_IMPL(0x0052c67c, FUN_0052c67c, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
