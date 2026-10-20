#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests the word at 0x009679a4 against 0x23. When equal the routine continues with `or eax, eax`,
// so the flags are those of that instruction; otherwise they are those of the `cmp`.
// FUNCTION: GP4 0x00510d7f
void __cdecl FUN_00510d7f(gp4::Regs* r) {
    const uint16_t value = GP4_GLOBAL(uint16_t, 0x009679a4);
    uint32_t flags = gp4::flags::sub16(value, 0x23u);
    if (value == 0x23u) {
        flags = gp4::flags::logic32(r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00510d7f, FUN_00510d7f, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
