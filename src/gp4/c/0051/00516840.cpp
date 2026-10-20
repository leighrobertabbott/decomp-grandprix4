#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores into 0x00a6987c: the record 0x0040eb0b finds for the selected car [0x00679d30] (a pointer
// into the 22 car records), after it passes the pair test 0x0040eb1f (selected car in ebx, found
// record in esi, CF clear); when no record is found or the pair is rejected (CF set) it stores
// 0x20000000. The stored value is eax after 0x0040eb1f (the compare routine rewrites eax).
// pushal/popal wrapper: every GPR is restored; the flags are those of "test ebx,ebx" when nothing was
// found, otherwise those 0x0040eb1f leaves.
// FUNCTION: GP4 0x00516840
void __cdecl FUN_00516840(gp4::Regs* r) {
    const uint32_t selected = GP4_GLOBAL(uint32_t, 0x00679d30);

    gp4::Regs find = *r;
    find.st_in = 0;
    find.st_out = 0;
    find.st_xin_mask = 0;
    find.st_xout_mask = 0;
    find.esi = selected;
    gp4::call_regs(0x0040eb0b, find);

    const uint32_t found = find.eax;
    uint32_t arith = gp4::flags::logic32(found);
    uint32_t stored = 0x20000000u;
    if (found != 0u) {
        gp4::Regs pair = find;
        pair.ebx = selected;
        pair.esi = found;
        gp4::call_regs(0x0040eb1f, pair);
        arith = pair.eflags;
        if ((pair.eflags & gp4::CF) == 0u) {
            stored = pair.eax;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = stored;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00516840, FUN_00516840, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+carno_0066ff6c+order_00418f8f")
