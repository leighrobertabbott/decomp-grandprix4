#include <gp4/gp4.h>
#include <gp4/flags.h>

// Replay-mode driver check. If the byte [0x007c2ea9] has its top bit clear it runs 0x0040617d with
// ax = 0x4a40, keeps its result in edi and sets the word pair [0x006f2c22] = 0x600, [0x006f2c20] = 0.
// Otherwise it sets [0x006f2c22] = 0x600, [0x006f2c20] = 0x200, derives in edi the track-segment record
// 0x2c0 bytes (two records) behind the lap start [0x00891504] (clamped at the pit start [0x00891500],
// wrapping by [0x00866ec4] below the lap start) and asks 0x00520023; when that leaves CF set and
// exactly two drivers are flagged (0x00522da0: eax + edx == 2) it clears [0x006f2c22], unless the byte
// [0x007c2e74] is set and 0x00522d18 leaves CF set while [0x007ab756] or [0x007a93b2] exceeds 2, or
// the byte [0x007c2e74] is clear and [0x007c2e7f] is 1.
// Registers: edi (see above), eax / edx as the last callee leaves them; the flags are those of the last
// compare or callee.
// FUNCTION: GP4 0x00522e54
void __cdecl FUN_00522e54(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(mode));

    if ((mode & 0x80u) == 0u) {
        c.eax = (c.eax & 0xffff0000u) | 0x4a40u;
        gp4::call_regs(0x0040617d, c);
        c.edi = c.eax;
        GP4_GLOBAL(uint16_t, 0x006f2c22) = 0x600u;
        GP4_GLOBAL(uint16_t, 0x006f2c20) = 0u;
        *r = c;
        return;
    }

    GP4_GLOBAL(uint16_t, 0x006f2c22) = 0x600u;
    GP4_GLOBAL(uint16_t, 0x006f2c20) = 0x200u;

    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00891504);
    uint32_t flags;
    if (edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        edi -= 0x2c0u;
        flags = gp4::flags::sub32(edi, GP4_GLOBAL(uint32_t, 0x00891500));
        if (edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            edi = GP4_GLOBAL(uint32_t, 0x00891500);
        }
    } else {
        edi -= 0x2c0u;
        flags = gp4::flags::sub32(edi, GP4_GLOBAL(uint32_t, 0x00891504));
        if (edi < GP4_GLOBAL(uint32_t, 0x00891504)) {
            const uint32_t wrap = GP4_GLOBAL(uint32_t, 0x00866ec4);
            flags = gp4::flags::add32(edi, wrap);
            edi += wrap;
        }
    }

    c.edi = edi;
    c.eflags = gp4::flags::merge(c.eflags, flags);
    gp4::call_regs(0x00520023, c);
    if ((c.eflags & 1u) == 0u) {
        *r = c;
        return;
    }

    gp4::call_regs(0x00522da0, c);
    const uint32_t sum = c.eax + c.edx;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.eax, c.edx));
    c.eax = sum;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(sum, 2u));
    if (sum != 2u) {
        *r = c;
        return;
    }

    const uint8_t live = GP4_GLOBAL(uint8_t, 0x007c2e74);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(live));
    if (live != 0u) {
        gp4::call_regs(0x00522d18, c);
        if ((c.eflags & 1u) != 0u) {
            const uint8_t first = GP4_GLOBAL(uint8_t, 0x007ab756);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(first, 2u));
            if (first > 2u) {
                *r = c;
                return;
            }
            const uint8_t second = GP4_GLOBAL(uint8_t, 0x007a93b2);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(second, 2u));
            if (second > 2u) {
                *r = c;
                return;
            }
        }
    } else {
        const uint8_t third = GP4_GLOBAL(uint8_t, 0x007c2e7f);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(third, 1u));
        if (third == 1u) {
            *r = c;
            return;
        }
    }
    GP4_GLOBAL(uint16_t, 0x006f2c22) = 0u;
    *r = c;
}
GP4_IMPL(0x00522e54, FUN_00522e54, "regs(eax:u32) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023+ring_008943a0")
