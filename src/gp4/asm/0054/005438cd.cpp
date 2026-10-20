#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x005438b8 with the step routine 0x00539c1d (increment, clamped to the limit) instead of
// 0x00539c04: for the menu events 2 and 3 (edx) it steps the value block at [ebx] and then runs the
// record check 0x005438fc, whose registers (eax, esi) and flags are the result. Any other event only
// leaves the flags of `cmp edx, 3`.
// FUNCTION: GP4 0x005438cd
void __cdecl FUN_005438cd(gp4::Regs* r) {
    const uint32_t event = r->edx;
    if (event == 2u || event == 3u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00539c1d, c);
        gp4::call_regs(0x005438fc, c);
        *r = c;
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 3u));
}
GP4_IMPL(0x005438cd, FUN_005438cd, "regs(eax:u32, ebx:rec[0x830b00,0x10,0..0], edx:u32{=2,3,0,1,4,5}) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sort_00542224+recs_005438a0+cur_00543942")
