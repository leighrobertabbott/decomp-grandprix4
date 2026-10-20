#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte at 0x0082ff13 is 0xff the (empty) routine 0x00531b40 is called. Both paths end
// with the flags of `cmp byte [0x82ff13], 0xff`: the callee is a bare `ret`.
// FUNCTION: GP4 0x0053ee3a
void __cdecl FUN_0053ee3a(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x0082ff13);
    if (value == 0xffu) {
        gp4::cdecl_<0x00531b40, void>();
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 0xffu));
}
GP4_IMPL(0x0053ee3a, FUN_0053ee3a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
