#include <gp4/gp4.h>

// Tail wrapper: `call 0x0053a393 ; ret`. The callee (a pushal/popal menu item drawer for the item
// record esi) restores every register; only the flags of its last call remain. It passes the entry
// registers on to the renderer calls, so they are inputs too.
// FUNCTION: GP4 0x0052974c
void __cdecl FUN_0052974c(gp4::Regs* r) {
    gp4::call_regs(0x0053a393, *r);
}
GP4_IMPL(0x0052974c, FUN_0052974c, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:layout=rec_005297ba+item_0053a393) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+script_0054d798+textout_0054d211")
