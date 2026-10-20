#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x005438b8 / 0x005438cd for the scroll-bar item handler 0x00539798: for the menu events 2 and 3
// (edx) it runs the scroll handler on the item at esi and then the record check 0x005438fc, whose
// registers and flags are the result. Any other event only leaves the flags of `cmp edx, 2`. edx is
// cleared on every path (`mov edx, 0`).
// FUNCTION: GP4 0x005438e2
void __cdecl FUN_005438e2(gp4::Regs* r) {
    const uint32_t event = r->edx;
    if (event == 2u || event == 3u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00539798, c);
        gp4::call_regs(0x005438fc, c);
        *r = c;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 2u));
    }
    r->edx = 0u;
}
GP4_IMPL(0x005438e2, FUN_005438e2, "regs(edx:u32{=2,3,0,1,4,5}, esi:ptr[0x20]:layout=bar_00539798, eax:u32, ebx:u32, ecx:u32, ebp:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=thumb_00539798+sort_00542224+recs_005438a0+cur_00543942")
