#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte at 0x007ad844 is 0xff the (empty) routine 0x0051d48d is called. Both paths end
// with the flags of `cmp byte [0x7ad844], 0xff`: the callee is a bare `ret`.
// FUNCTION: GP4 0x0051d47a
void __cdecl FUN_0051d47a(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x007ad844);
    if (value == 0xffu) {
        gp4::cdecl_<0x0051d48d, void>();
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 0xffu));
}
GP4_IMPL(0x0051d47a, FUN_0051d47a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
