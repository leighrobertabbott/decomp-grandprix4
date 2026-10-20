#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-tick update of a block of word counters/steppers around 0x008705ae..0x00870616 and the two
// dword steppers at 0x00874dd8..0x00874dec: increments the word 0x008705e8 and the byte 0x008705c0,
// adds the velocity words 0x008705b2/b4 to 0x008705ae/b0, and for each nonzero down-counter
// (0x00870602, 0x00870606, 0x00870614, 0x00874de0, 0x00874de4) decrements it and adds its step to
// the matching value (0x00870604 also refreshes 0x0087060a = value + 0x50; 0x00870610 snaps to
// 0x00870616 when its counter reaches zero). Register code: eax holds the last loaded value (ax
// for the word parts) and the flags are those of the last test/add/sub.
// FUNCTION: GP4 0x0055a636
void __cdecl FUN_0055a636(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t arith;

    GP4_GLOBAL(uint16_t, 0x008705e8) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008705e8) + 1u);
    GP4_GLOBAL(uint8_t, 0x008705c0) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008705c0) + 1u);

    uint16_t ax = GP4_GLOBAL(uint16_t, 0x008705b2);
    GP4_GLOBAL(uint16_t, 0x008705ae) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008705ae) + ax);
    ax = GP4_GLOBAL(uint16_t, 0x008705b4);
    GP4_GLOBAL(uint16_t, 0x008705b0) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x008705b0) + ax);

    if (GP4_GLOBAL(uint16_t, 0x00870602) != 0u) {
        GP4_GLOBAL(uint16_t, 0x00870602) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870602) - 1u);
        ax = GP4_GLOBAL(uint16_t, 0x0087060c);
        GP4_GLOBAL(uint16_t, 0x00870604) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870604) + ax);
        ax = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870604) + 0x50u);
        GP4_GLOBAL(uint16_t, 0x0087060a) = ax;
    }

    if (GP4_GLOBAL(uint16_t, 0x00870606) != 0u) {
        GP4_GLOBAL(uint16_t, 0x00870606) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870606) - 1u);
        ax = GP4_GLOBAL(uint16_t, 0x0087060e);
        GP4_GLOBAL(uint16_t, 0x00870608) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870608) + ax);
    }

    if (GP4_GLOBAL(uint16_t, 0x00870614) != 0u) {
        ax = GP4_GLOBAL(uint16_t, 0x00870612);
        GP4_GLOBAL(uint16_t, 0x00870610) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870610) + ax);
        GP4_GLOBAL(uint16_t, 0x00870614) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870614) - 1u);
        if (GP4_GLOBAL(uint16_t, 0x00870614) == 0u) {
            ax = GP4_GLOBAL(uint16_t, 0x00870616);
            GP4_GLOBAL(uint16_t, 0x00870610) = ax;
        }
    }
    eax = (eax & 0xffff0000u) | ax;

    if (GP4_GLOBAL(uint32_t, 0x00874de0) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00874de0) -= 1u;
        eax = GP4_GLOBAL(uint32_t, 0x00874de8);
        GP4_GLOBAL(uint32_t, 0x00874dd8) += eax;
    }

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00874de4);
    if (count != 0u) {
        GP4_GLOBAL(uint32_t, 0x00874de4) = count - 1u;
        eax = GP4_GLOBAL(uint32_t, 0x00874dec);
        const uint32_t old = GP4_GLOBAL(uint32_t, 0x00874ddc);
        GP4_GLOBAL(uint32_t, 0x00874ddc) = old + eax;
        arith = gp4::flags::add32(old, eax);
    } else {
        arith = gp4::flags::logic32(0u);
    }

    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055a636, FUN_0055a636, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
