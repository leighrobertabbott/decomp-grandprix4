#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reserves 0x800 bytes from the block pool (0x005389b9); on success (eax != 0) the returned address
// is stored in 0x00839169. Then 0x0054f753 resets the 34-entry table at 0x00838ec1. eax is
// saved and restored (push eax / pop eax); the flags are those the table reset leaves.
// FUNCTION: GP4 0x0054f5e9
void __cdecl FUN_0054f5e9(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = 0x800u;
    gp4::call_regs(0x005389b9, c);
    if (c.eax != 0u) {
        GP4_GLOBAL(uint32_t, 0x00839169) = c.eax;
    }
    gp4::call_regs(0x0054f753, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054f5e9, FUN_0054f5e9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pool_005389b9+pool_0054f5e9")
