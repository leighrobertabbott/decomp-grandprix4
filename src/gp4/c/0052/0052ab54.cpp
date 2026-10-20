#include <gp4/gp4.h>

// `call 0x0052d5f8 ; ret`: runs the menu item handler (edx = event, esi = item); registers and flags come
// straight back from it.
// FUNCTION: GP4 0x0052ab54
void __cdecl FUN_0052ab54(gp4::Regs* r) {
    gp4::call_regs(0x0052d5f8, *r);
}
GP4_IMPL(0x0052ab54, FUN_0052ab54, "regs(edx:u32{=2,2,0,1,3}, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0052d5f8")
