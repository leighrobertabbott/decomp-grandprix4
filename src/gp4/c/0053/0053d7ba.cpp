#include <gp4/gp4.h>

// Menu navigation shortcut: ebx = 0x25 selects the link byte at +0x25 of the menu item in esi, then the
// shared walker 0x0053dca6 runs (call + ret, so it behaves as a tail call).
// FUNCTION: GP4 0x0053d7ba
void __cdecl FUN_0053d7ba(gp4::Regs* r) {
    r->ebx = 0x25u;
    gp4::call_regs(0x0053dca6, *r);
}
GP4_IMPL(0x0053d7ba, FUN_0053d7ba, "regs(ecx:u32, edx:u32, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053d8c5+items_0053dc57+links_0053dca6")
