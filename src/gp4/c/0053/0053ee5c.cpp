#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the (unaligned) dword 0xce at 0x0082ff0f and runs 0x00549d12 under pushal/popal: every register is
// kept and the flags are those 0x00549d12 leaves (popal restores none).
// FUNCTION: GP4 0x0053ee5c
void __cdecl FUN_0053ee5c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    GP4_GLOBAL(uint32_t, 0x0082ff0f) = 0xceu;
    gp4::call_regs(0x00549d12, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053ee5c, FUN_0053ee5c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
