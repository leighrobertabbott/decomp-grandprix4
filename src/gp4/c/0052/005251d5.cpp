#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: finds the index (0 .. [0x006f3ad8]-1) whose table entry 0x005250f8 puts closest to the
// selected car (smallest lap distance, ties to the later index): 0x005250f8 is called with ebx = index and
// leaves ecx = distance and CF = 1 when either position is beyond the lap limit; those indexes are skipped.
// The best index (edi, starting at 0) goes to [0x00679bd8]. popal restores every GPR; the flags are those of
// the last `cmp ebx, [0x006f3ad8]`.
// FUNCTION: GP4 0x005251d5
void __cdecl FUN_005251d5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    int32_t best = 0x7fffffff;
    uint32_t found = 0u;
    uint32_t index = 0u;
    uint32_t flags;
    do {
        c.ebx = index;
        gp4::call_regs(0x005250f8, c);
        if ((c.eflags & gp4::flags::CF) == 0u && static_cast<int32_t>(c.ecx) <= best) {
            best = static_cast<int32_t>(c.ecx);
            found = index;
        }
        ++index;
        flags = gp4::flags::sub32(index, GP4_GLOBAL(uint32_t, 0x006f3ad8));
    } while (static_cast<int32_t>(index) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f3ad8)));

    GP4_GLOBAL(uint32_t, 0x00679bd8) = found;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005251d5, FUN_005251d5, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+ptrs_006f2f6c+ring_00525205+pos_00525073")
