#include <gp4/gp4.h>
#include <gp4/flags.h>

// `inc eax` then a tail call into the table lookup 0x005484f2 (index of the first of the 22 bytes at
// 0x7c2a48 whose low six bits equal al, halved), whose result and flags are returned unchanged.
// FUNCTION: GP4 0x005484eb
void __cdecl FUN_005484eb(gp4::Regs* r) {
    const uint32_t value = r->eax;
    r->eax = value + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(value, r->eflags));
    gp4::call_regs(0x005484f2, *r);
}
GP4_IMPL(0x005484eb, FUN_005484eb, "regs(eax:u32[0..0x3f]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
