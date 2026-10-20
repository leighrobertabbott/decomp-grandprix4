#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054f604
// push eax / pop eax around: resets the 34-entry table 0x00838ec1 (0x0054f753); when the dword
// [0x839169] (the 0x800-byte pool block taken by 0x0054f5e9) is nonzero, gives 0x800 bytes back
// (0x00538a24 with eax = 0x800) and clears it. eax is unchanged; the flags are those of
// `cmp dword [0x839169], 0` when it is zero, else those 0x00538a24 leaves.
void __cdecl FUN_0054f604(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054f753, c);

    const uint32_t block = GP4_GLOBAL(uint32_t, 0x00839169);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(block, 0u));
    if (block != 0u) {
        c.eax = 0x800u;
        gp4::call_regs(0x00538a24, c);
        GP4_GLOBAL(uint32_t, 0x00839169) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054f604, FUN_0054f604, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pool_0054f5e9+pool_00538a24+tbl_00838ec1")
