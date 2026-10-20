#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu handler for the event 2 (edx): runs the menu item handler 0x0052d5f8 (esi = item, ebx = its
// value cell, read by 0x0052d617 through 0x00539944), then 0x0052bae5, and leaves edx = 2. Any other
// event only leaves the flags of `cmp edx, 2`. The event-2 chain posts a window message and pumps
// messages (0x46ae90 -> 0x5086b0, PeekMessageA), so that arm is checked in game.
// FUNCTION: GP4 0x0052befa
void __cdecl FUN_0052befa(gp4::Regs* r) {
    const uint32_t event = r->edx;
    if (event == 2u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 2u));
        gp4::call_regs(0x0052d5f8, c);
        gp4::call_regs(0x0052bae5, c);
        c.edx = 2u;
        *r = c;
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 2u));
}
GP4_IMPL(0x0052befa, FUN_0052befa, "regs(edx:u32{=2,2,0,1,3}, ebx:ptr[8]:bytes, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0052d5f8; emu=skip:win32 PeekMessageA")
