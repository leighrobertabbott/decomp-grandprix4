#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the 3-bit counter byte at 0x007aafd0 (wraps 7 -> 0) through al; the low byte of eax
// ends up holding the new value and the flags are those of `and al, 7`.
// FUNCTION: GP4 0x0054ba6b
void __cdecl FUN_0054ba6b(gp4::Regs* r) {
    uint8_t al = GP4_GLOBAL(uint8_t, 0x007aafd0);
    al = static_cast<uint8_t>(al + 1u);
    al = static_cast<uint8_t>(al & 7u);
    GP4_GLOBAL(uint8_t, 0x007aafd0) = al;
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(al));
}
GP4_IMPL(0x0054ba6b, FUN_0054ba6b, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
