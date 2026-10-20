#include <gp4/gp4.h>

// Sets the current directory to the drive root string "C:\" (0x0081e7f2) through 0x005368c3: eax is the
// BOOL result and the flags are those 0x005368c3 leaves (`jb` then `clc`/`stc` rewrite CF with the
// value it already had).
// FUNCTION: GP4 0x005367a8
void __cdecl FUN_005367a8(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x0081e7f2)));
    gp4::call_regs(0x005368c3, c);
    r->eax = c.eax;
    r->eflags = c.eflags;
}
GP4_IMPL(0x005367a8, FUN_005367a8, "regs() -> (eax, cf, pf, af, zf, sf, of)")
