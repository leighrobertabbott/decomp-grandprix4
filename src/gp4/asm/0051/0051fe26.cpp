#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051fe26
// Like 0x0051fd6f (binary to packed BCD of eax, signed compares) but the top digit counts
// 6000000 (100 minutes in ms): 6000000 -> 0x10000000, 600000 -> 0x1000000, 60000 -> 0x100000,
// 10000 -> 0x10000, 1000 -> 0x1000, 100 -> 0x100, 10 -> 0x10, 1 -> 1. A non-positive input stays
// as is and yields 0. edx is saved and restored. Flags are those of the final `cmp eax, 1`
// on the remainder.
void __cdecl FUN_0051fe26(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(r->eax);
    uint32_t packed = 0;

    while (value >= 6000000) {
        value -= 6000000;
        packed += 0x10000000u;
    }
    while (value >= 600000) {
        value -= 600000;
        packed += 0x1000000u;
    }
    while (value >= 60000) {
        value -= 60000;
        packed += 0x100000u;
    }
    while (value >= 10000) {
        value -= 10000;
        packed += 0x10000u;
    }
    while (value >= 1000) {
        value -= 1000;
        packed += 0x1000u;
    }
    while (value >= 100) {
        value -= 100;
        packed += 0x100u;
    }
    while (value >= 10) {
        value -= 10;
        packed += 0x10u;
    }
    while (value >= 1) {
        value -= 1;
        packed += 1u;
    }

    r->eax = packed;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(static_cast<uint32_t>(value), 1u));
}
GP4_IMPL(0x0051fe26, FUN_0051fe26, "regs(eax:i32[-100..30000000]) -> (eax, cf, pf, af, zf, sf, of)")
