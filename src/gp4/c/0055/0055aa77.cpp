#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clamps a pair of values against the limits [0x866ed0]/[0x866ed4]: [0x868520] = the larger (unsigned) of
// [0x868518] and [0x866ed0]; [0x868524] = the smaller (unsigned) of [0x86851c] and [0x866ed4]. eax ends as
// the second result and the flags are those of the final `cmp eax, [0x866ed4]`.
// FUNCTION: GP4 0x0055aa77
void __cdecl FUN_0055aa77(gp4::Regs* r) {
    uint32_t value = GP4_GLOBAL(uint32_t, 0x00868518);
    if (value < GP4_GLOBAL(uint32_t, 0x00866ed0)) {
        value = GP4_GLOBAL(uint32_t, 0x00866ed0);
    }
    GP4_GLOBAL(uint32_t, 0x00868520) = value;

    value = GP4_GLOBAL(uint32_t, 0x0086851c);
    const uint32_t flags = gp4::flags::sub32(value, GP4_GLOBAL(uint32_t, 0x00866ed4));
    if (value > GP4_GLOBAL(uint32_t, 0x00866ed4)) {
        value = GP4_GLOBAL(uint32_t, 0x00866ed4);
    }
    GP4_GLOBAL(uint32_t, 0x00868524) = value;

    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055aa77, FUN_0055aa77, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
