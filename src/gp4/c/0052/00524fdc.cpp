#include <gp4/gp4.h>
#include <gp4/flags.h>

// Steps the selection index [0x679bd8] over a list of [0x6f3ad8] entries, driven by the key byte
// [0x66d7a6]: bit 0x80 moves forward (wrapping to 0 at the count), bit 0x40 moves back (wrapping to
// count - 1 from 0), otherwise the index is unchanged. eax ends up holding the stored index and
// the flags are those of the last flag writer on each path (test / cmp / dec after the cmp).
// FUNCTION: GP4 0x00524fdc
void __cdecl FUN_00524fdc(gp4::Regs* r) {
    uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    const uint32_t keys = GP4_GLOBAL(uint8_t, 0x0066d7a6);
    uint32_t f = gp4::flags::logic8(keys & 0x80u);
    if ((keys & 0x80u) != 0u) {
        index += 1u;
        f = gp4::flags::sub32(index, GP4_GLOBAL(uint32_t, 0x006f3ad8));
        if ((f & gp4::flags::CF) == 0u) {
            index = 0u;
        }
    } else {
        f = gp4::flags::logic8(keys & 0x40u);
        if ((keys & 0x40u) != 0u) {
            index -= 1u;
            f = gp4::flags::sub32(index, GP4_GLOBAL(uint32_t, 0x006f3ad8));
            if ((f & gp4::flags::CF) == 0u) {
                const uint32_t count = GP4_GLOBAL(uint32_t, 0x006f3ad8);
                index = count - 1u;
                f = gp4::flags::dec32(count, f);
            }
        }
    }
    GP4_GLOBAL(uint32_t, 0x00679bd8) = index;
    r->eax = index;
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00524fdc, FUN_00524fdc, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
