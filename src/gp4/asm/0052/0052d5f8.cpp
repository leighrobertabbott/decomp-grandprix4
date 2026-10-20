#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item handler for the event edx = 2 (activation): runs the record setup 0x0052d617 (esi = item), the
// empty routine 0x0052d66f, the redraw 0x0052d670 and 0x0052d6d0, then returns edx = 5. Any other event
// only leaves the flags of `cmp edx, 2`.
// FUNCTION: GP4 0x0052d5f8
void __cdecl FUN_0052d5f8(gp4::Regs* r) {
    const uint32_t event = r->edx;
    if (event == 2u) {
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 2u));
        gp4::call_regs(0x0052d617, c);
        gp4::cdecl_<0x0052d66f, void>();
        gp4::call_regs(0x0052d670, c);
        gp4::call_regs(0x0052d6d0, c);
        c.edx = 5u;
        *r = c;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 2u));
    }
}
GP4_IMPL(0x0052d5f8, FUN_0052d5f8, "regs(edx:u32{=2,2,0,1,3}, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0052d5f8")
