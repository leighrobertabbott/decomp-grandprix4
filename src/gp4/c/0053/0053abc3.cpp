#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs the menu-item "queue and style" routine 0x0053aab9 with edx = 0x37 and
// restores every register with popal. popal does not restore the flags, so the flags are those the
// callee leaves.
// FUNCTION: GP4 0x0053abc3
void __cdecl FUN_0053abc3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edx = 0x37u;
    gp4::call_regs(0x0053aab9, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053abc3, FUN_0053abc3, "regs(esi:ptr[0x40]:u8{0,0x10,0x20,0x30,0x80,0x90,0xa0,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+sel_0053aab9")
