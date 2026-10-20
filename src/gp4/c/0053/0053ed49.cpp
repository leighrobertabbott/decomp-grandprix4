#include <gp4/gp4.h>

// `call 0x005391b0 ; ret`: the menu item value handler for the command in edx on the item in esi (ebx =
// its value cell). Registers and flags come straight back from the call.
// FUNCTION: GP4 0x0053ed49
void __cdecl FUN_0053ed49(gp4::Regs* r) {
    gp4::call_regs(0x005391b0, *r);
}
GP4_IMPL(0x0053ed49, FUN_0053ed49, "regs(eax:u32, edx:u32{1,2,3,0,2,3,4}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,0,0,0,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
