#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x007a9794 is 0xff, stores al into the byte at 0x00822272. The flags are those of
// `cmp byte [0x7a9794], 0xff`; no register is changed.
// FUNCTION: GP4 0x0053ee02
void __cdecl FUN_0053ee02(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x007a9794);
    if (value == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x00822272) = static_cast<uint8_t>(r->eax & 0xffu);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 0xffu));
}
GP4_IMPL(0x0053ee02, FUN_0053ee02, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
