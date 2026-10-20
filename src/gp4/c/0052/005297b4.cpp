#include <gp4/gp4.h>

// Tail wrapper: `call 0x0053a620 ; ret`. The callee (a pushal/popal menu item drawer for the item
// record esi, the 5x scaled variant of 0x0053a393) restores every register; only the flags of its
// last call remain. It passes the entry registers on to the renderer calls, so they are inputs too.
// FUNCTION: GP4 0x005297b4
void __cdecl FUN_005297b4(gp4::Regs* r) {
    gp4::call_regs(0x0053a620, *r);
}
GP4_IMPL(0x005297b4, FUN_005297b4, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:layout=rec_005297ba+item_0053a393) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+script_0054d798+textout_0054d211")
