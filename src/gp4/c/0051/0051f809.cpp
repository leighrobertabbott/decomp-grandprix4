#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the byte at 0x0088fdd7 with 0xff, then `stc` (equal) or `clc` (not equal): the other
// flags are those of the `cmp byte`, CF is replaced by the result of the test.
// FUNCTION: GP4 0x0051f809
void __cdecl FUN_0051f809(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x0088fdd7);
    const uint32_t cmp = gp4::flags::sub8(value, 0xffu);
    const uint32_t flags = (cmp & ~1u) | (value == 0xffu ? 1u : 0u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051f809, FUN_0051f809, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
