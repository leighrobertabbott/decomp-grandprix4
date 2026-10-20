#include <gp4/gp4.h>
#include <gp4/flags.h>

// `pushal ; walk 0x008943a0 .. [0x866ecc] ; walk [0x866ed0] .. [0x866ed4] ; popal ; ret`: runs the
// segment draw-setup driver 0x005522fa twice over two ranges of 0x160-byte ring records (edi = first
// record, ebx = end). popal restores every register; only the flags of the last compare remain. The
// entry edi is only an input to the record-pointer table the draw setup aliases it with.
// FUNCTION: GP4 0x005523ce
void __cdecl FUN_005523ce(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x008943a0)));
    c.ebx = GP4_GLOBAL(uint32_t, 0x00866ecc);
    gp4::call_regs(0x005522fa, c);
    c.edi = GP4_GLOBAL(uint32_t, 0x00866ed0);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00866ed4);
    gp4::call_regs(0x005522fa, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005523ce, FUN_005523ce, "regs(edi:rec[0x8943a0,0x160,0..6]) -> (cf, pf, af, zf, sf, of); globals=layout=geom_008943a0+span_00551227+mask_005520a0+ring_00555b28+state_00552126+rows_0055218f+ends_005523ce")
