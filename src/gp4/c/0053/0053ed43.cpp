#include <gp4/gp4.h>

// `call 0x005394b1 ; ret`: the menu item value handler for the command in edx on the item in esi. Registers and
// flags come straight back from the call.
// FUNCTION: GP4 0x0053ed43
void __cdecl FUN_0053ed43(gp4::Regs* r) {
    gp4::call_regs(0x005394b1, *r);
}
GP4_IMPL(0x0053ed43, FUN_0053ed43, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,4,5,6,0,4,2,3}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+flag4_005394b1")
