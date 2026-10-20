#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reports through the carry flag whether the dword at 0x007ad87c is non-zero: CF = 0 (clc) when it
// is zero, CF = 1 (stc) otherwise. The other flags are those of `cmp dword [0x7ad87c], 0`.
// FUNCTION: GP4 0x00531743
void __cdecl FUN_00531743(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007ad87c);
    const uint32_t carry = (value != 0u) ? 1u : 0u;
    const uint32_t flags = (gp4::flags::sub32(value, 0u) & ~1u) | carry;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00531743, FUN_00531743, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
