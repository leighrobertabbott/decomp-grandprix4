#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares eax with the dword at 0x00932464. When they differ the flags are those of the `cmp`;
// when equal the routine falls into `test dword [0x00875278], 0xffffffff` and leaves its flags.
// FUNCTION: GP4 0x00526380
void __cdecl FUN_00526380(gp4::Regs* r) {
    const uint32_t value = r->eax;
    const uint32_t other = GP4_GLOBAL(uint32_t, 0x00932464);
    uint32_t arith = gp4::flags::sub32(value, other);
    if (value == other) {
        arith = gp4::flags::logic32(GP4_GLOBAL(uint32_t, 0x00875278));
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00526380, FUN_00526380, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
