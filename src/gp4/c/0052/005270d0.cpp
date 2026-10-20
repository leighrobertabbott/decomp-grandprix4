#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005270d0
// Re-locates the ring record [0x679b78] for the position ([0x7a8dd4] << 5, [0x7a8ddc] << 5) with the
// walker 0x00405da9 (eax = x, edx = z in the fixed-point scale, edi = the record) and stores the
// record it returns back in [0x679b78]. eax and edi leave as that record, edx as [0x7a8ddc] << 5;
// the flags are those the walker leaves.
void __cdecl FUN_005270d0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edi = GP4_GLOBAL(uint32_t, 0x00679b78);
    c.eax = GP4_GLOBAL(uint32_t, 0x007a8dd4) << 5;
    c.edx = GP4_GLOBAL(uint32_t, 0x007a8ddc) << 5;
    gp4::call_regs(0x00405da9, c);
    GP4_GLOBAL(uint32_t, 0x00679b78) = c.eax;
    r->eax = c.eax;
    r->edx = c.edx;
    r->edi = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005270d0, FUN_005270d0, "regs() -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00405a12+view_005270d0")
