#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: asks 0x00408359 for the car record whose number byte +0x7c equals the low byte of
// [0x00a69878]; [0x00a6987c] gets the record address when one is found (CF set) and 0 otherwise. Every GPR is
// restored; the flags are those 0x00408359 leaves.
// FUNCTION: GP4 0x005164ba
void __cdecl FUN_005164ba(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x00408359, c);
    if ((c.eflags & static_cast<uint32_t>(gp4::flags::CF)) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    } else {
        GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005164ba, FUN_005164ba, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+carno_0066ff6c+idx_005162fd")
