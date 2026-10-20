#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatcher on the low byte of eax (values above 7 count as 7) through the 8-entry jump table
// 0x0053683c: modes 1..4 run the directory setter 0x005367b9, mode 6 runs 0x005367a8, and modes 0, 5
// and 7 run the empty routine 0x0054e232 (esi = the code tag 0x00536865 for 0 and 5, 0x00536898 for 7)
// inside a pushal/popal. ebx is saved and restored. eax and the flags come from the chosen routine
// (flags enter it as those of `add ebx, 0x53683c`); in the empty cases only those flags remain.
// FUNCTION: GP4 0x00536823
void __cdecl FUN_00536823(gp4::Regs* r) {
    uint32_t mode = r->eax & 0xffu;
    if (mode >= 8u) {
        mode = 7u;
    }
    const uint32_t slot = mode << 2;
    const uint32_t entryFlags = gp4::flags::add32(slot, 0x0053683cu);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ebx = slot + 0x0053683cu;
    c.eflags = gp4::flags::merge(r->eflags, entryFlags);

    if (mode >= 1u && mode <= 4u) {
        gp4::call_regs(0x005367b9, c);
        r->eax = c.eax;
        r->eflags = c.eflags;
    } else if (mode == 6u) {
        gp4::call_regs(0x005367a8, c);
        r->eax = c.eax;
        r->eflags = c.eflags;
    } else {
        c.esi = (mode == 7u) ? 0x00536898u : 0x00536865u;
        gp4::call_regs(0x0054e232, c);
        r->eflags = gp4::flags::merge(r->eflags, entryFlags);
    }
}
GP4_IMPL(0x00536823, FUN_00536823, "regs(eax:u32{0,1,2,3,4,5,6,7,8,0xff}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
