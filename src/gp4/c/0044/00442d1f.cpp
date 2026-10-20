#include <gp4/gp4.h>

// FUNCTION: GP4 0x00442d1f
// Hands the remaining time to the clock-digit writer 0x0051f00d: eax = [0x66d83c] - [0x66d814] + [0x62ad74],
// clamped to 1000..5000 (signed). pushal / popal restore every register; the flags are those 0x0051f00d leaves.
void __cdecl FUN_00442d1f(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d83c) - GP4_GLOBAL(uint32_t, 0x0066d814)
                                         + GP4_GLOBAL(uint32_t, 0x0062ad74));
    if (value > 0x1388) {
        value = 0x1388;
    }
    if (value < 0x3e8) {
        value = 0x3e8;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = static_cast<uint32_t>(value);
    gp4::call_regs(0x0051f00d, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00442d1f, FUN_00442d1f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00442d1f")
