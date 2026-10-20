#include <gp4/gp4.h>

// Tail wrapper: `call 0x0053a393 ; ret`. The callee (pushal/popal) draws the menu item at esi
// (colour, text from the id at +0x12, value cell at +0xe); registers come back unchanged and the
// flags are those of its last flag writer.
// FUNCTION: GP4 0x00529746
void __cdecl FUN_00529746(gp4::Regs* r) {
    gp4::call_regs(0x0053a393, *r);
}
GP4_IMPL(0x00529746, FUN_00529746, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:layout=rec_005297ba+item_0053a393) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+script_0054d798")
