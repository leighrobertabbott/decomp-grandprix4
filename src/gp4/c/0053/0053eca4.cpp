#include <gp4/gp4.h>

// `call 0x0052d5f8 ; call 0x0053ecaf ; ret`: the menu item handler for the event in edx on the item in
// esi (0x0052d5f8: activation redraw for event 2), followed by the item-state update 0x0053ecaf. The
// registers and flags carry from the first call into the second and come back from the second.
// FUNCTION: GP4 0x0053eca4
void __cdecl FUN_0053eca4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052d5f8, c);
    gp4::call_regs(0x0053ecaf, c);
    *r = c;
}
GP4_IMPL(0x0053eca4, FUN_0053eca4, "regs(edx:u32{=2,2,0,1,3}, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0052d5f8; emu=skip:win32 PeekMessageA")
