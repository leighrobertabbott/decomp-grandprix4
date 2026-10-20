#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) void pass_low_first(gp4::Regs& c) {
    gp4::call_regs(0x00522e0e, c);
    gp4::call_regs(0x00522e31, c);
}

static __declspec(noinline) void pass_high_first(gp4::Regs& c) {
    gp4::call_regs(0x00522e31, c);
    gp4::call_regs(0x00522e0e, c);
}

// Drivers' slot-list refresh: when 0x00522d18 reports CF (few enough pending drivers), the byte
// [0x007aafd4] selects the single-pass refresh 0x00522e0e (zero) or the two-pass one. The two passes,
// 0x00522e0e (bit 7 only) and 0x00522e31 (bits 7 and 6), run in the order given by the two class bytes
// [0x007ab756] (a) and [0x007a93b2] (b): 0x00522e0e then 0x00522e31 when a < b, the reverse when a > b;
// when they are equal the pair is reset to (1,2) if [0x007ab1a0] == 0xff, else (2,1), and compared again.
// Registers and flags are those the last routine left; dl is reloaded from [0x007ab756].
// FUNCTION: GP4 0x00522d39
void __cdecl FUN_00522d39(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00522d18, c);
    if ((c.eflags & 1u) == 0u) {
        *r = c;
        return;
    }

    const uint32_t two_pass = GP4_GLOBAL(uint8_t, 0x007aafd4);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(two_pass));
    if (two_pass == 0u) {
        gp4::call_regs(0x00522e0e, c);
        *r = c;
        return;
    }

    for (;;) {
        const uint32_t mine = GP4_GLOBAL(uint8_t, 0x007ab756);
        const uint32_t other = GP4_GLOBAL(uint8_t, 0x007a93b2);
        c.edx = (c.edx & 0xffffff00u) | mine;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(mine, other));
        if (mine < other) {
            pass_low_first(c);
            break;
        }
        if (mine > other) {
            pass_high_first(c);
            break;
        }
        const uint32_t marker = GP4_GLOBAL(uint8_t, 0x007ab1a0);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(marker, 0xffu));
        if (marker == 0xffu) {
            GP4_GLOBAL(uint8_t, 0x007ab756) = 1u;
            GP4_GLOBAL(uint8_t, 0x007a93b2) = 2u;
        } else {
            GP4_GLOBAL(uint8_t, 0x007ab756) = 2u;
            GP4_GLOBAL(uint8_t, 0x007a93b2) = 1u;
        }
    }
    *r = c;
}
GP4_IMPL(0x00522d39, FUN_00522d39, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023+order_00675958+state_00522d39")
