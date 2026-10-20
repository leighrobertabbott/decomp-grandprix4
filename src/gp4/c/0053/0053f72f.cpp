#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the byte state 0x007ab020 to 0 when it is 5. The flags are those of `cmp byte [0x7ab020], 5`.
// FUNCTION: GP4 0x0053f72f
void __cdecl FUN_0053f72f(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x007ab020);
    if (value == 5u) {
        GP4_GLOBAL(uint8_t, 0x007ab020) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 5u));
}
GP4_IMPL(0x0053f72f, FUN_0053f72f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
