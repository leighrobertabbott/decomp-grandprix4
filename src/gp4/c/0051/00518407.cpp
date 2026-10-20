#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts a percentage byte through 0x5183e6 (clamp 0..100, Q14 = (p << 14) / 100) and returns the
// result; `flags` receives the flags that routine leaves.
static inline uint32_t Percent(uint8_t value, uint32_t& flags) {
    gp4::Regs c = {};
    c.eax = value;
    gp4::call_regs(0x005183e6, c);
    flags = c.eflags;
    return c.eax;
}

// pushal/popal routine that converts six percentage bytes of the settings block at 0x7ab182.. to
// Q14 words: 0x88f6e4/e8/ec/f0 from 0x7ab182/186/187/189, 0x88f6f4 from 0x7ab18b and, when the
// byte [0x7ad868] is zero, again from 0x7ab18a. Every register is preserved; the flags are those of
// `cmp byte [0x7ad868], 0` when it is nonzero, otherwise those the last conversion leaves.
// FUNCTION: GP4 0x00518407
void __cdecl FUN_00518407(gp4::Regs* r) {
    uint32_t flags = 0u;
    GP4_GLOBAL(uint32_t, 0x0088f6e4) = Percent(GP4_GLOBAL(uint8_t, 0x007ab182), flags);
    GP4_GLOBAL(uint32_t, 0x0088f6e8) = Percent(GP4_GLOBAL(uint8_t, 0x007ab186), flags);
    GP4_GLOBAL(uint32_t, 0x0088f6ec) = Percent(GP4_GLOBAL(uint8_t, 0x007ab187), flags);
    GP4_GLOBAL(uint32_t, 0x0088f6f0) = Percent(GP4_GLOBAL(uint8_t, 0x007ab189), flags);
    GP4_GLOBAL(uint32_t, 0x0088f6f4) = Percent(GP4_GLOBAL(uint8_t, 0x007ab18b), flags);

    const uint8_t keep = GP4_GLOBAL(uint8_t, 0x007ad868);
    flags = gp4::flags::sub8(keep, 0u);
    if (keep == 0u) {
        GP4_GLOBAL(uint32_t, 0x0088f6f4) = Percent(GP4_GLOBAL(uint8_t, 0x007ab18a), flags);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00518407, FUN_00518407, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
