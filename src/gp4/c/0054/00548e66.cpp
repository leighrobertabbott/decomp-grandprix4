#include <gp4/gp4.h>
#include <gp4/flags.h>

// Number of places to show in the standings: [0x831a61] = the number of drivers flagged in the
// 0x007c2a48 table (singles + pairs, 0x0052ceef) when the option byte [0x007ad8b2] is set and that sum
// exceeds 1; otherwise the number of cars whose state byte (0x007c2a74) is 1 (0x00548ec5) limited to
// 0x16 and raised to at least 1. eax is that value, edx keeps the second result of 0x0052ceef when it ran,
// and the flags are those of the final `cmp eax, 1` (on the value before the raise to 1).
// FUNCTION: GP4 0x00548e66
void __cdecl FUN_00548e66(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t value = 0u;
    bool direct = false;
    if (GP4_GLOBAL(uint8_t, 0x007ad8b2) != 0u) {
        gp4::call_regs(0x0052ceef, c);
        value = c.eax + c.edx;
        direct = value > 1u;
    }
    if (!direct) {
        gp4::call_regs(0x00548ec5, c);
        value = c.eax;
        if (value > 0x16u) {
            value = 0x16u;
        }
    }
    const uint32_t flags = gp4::flags::sub32(value, 1u);
    if (value < 1u) {
        value = 1u;
    }
    GP4_GLOBAL(uint32_t, 0x00831a61) = value;

    c.eax = value;
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, flags);
}
GP4_IMPL(0x00548e66, FUN_00548e66, "regs(edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3+tabs_0043ced2")
