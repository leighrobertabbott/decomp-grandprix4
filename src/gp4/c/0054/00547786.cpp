#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets [0x00831ad8] to 0 when the byte [0x007ad8b2] is 0xff and 0x0052ceef's two driver counts (eax,
// edx) add up to exactly 2 and then: with [0x007ad898] == 0, either count is 2 or both bytes
// [0x007ab756] and [0x007a93b2] are 2 or less; with [0x007ad898] != 0, the byte [0x007c2e7f] is not 1.
// Otherwise it is set to 0xff. pushal/popal restore every register; the flags are those of the last
// compare executed.
// FUNCTION: GP4 0x00547786
void __cdecl FUN_00547786(gp4::Regs* r) {
    uint32_t flags = r->eflags;
    bool clear = false;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ad8b2);
    flags = gp4::flags::merge(flags, gp4::flags::sub8(mode, 0xffu));
    if (mode == 0xffu) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x0052ceef, c);
        const uint32_t total = c.eax + c.edx;
        flags = gp4::flags::merge(flags, gp4::flags::sub32(total, 2u));
        if (total == 2u) {
            const uint32_t sixth = GP4_GLOBAL(uint32_t, 0x007ad898);
            flags = gp4::flags::merge(flags, gp4::flags::sub32(sixth, 0u));
            if (sixth == 0u) {
                flags = gp4::flags::merge(flags, gp4::flags::sub32(c.eax, 2u));
                if (c.eax == 2u) {
                    clear = true;
                } else {
                    flags = gp4::flags::merge(flags, gp4::flags::sub32(c.edx, 2u));
                    if (c.edx == 2u) {
                        clear = true;
                    } else {
                        const uint8_t a = GP4_GLOBAL(uint8_t, 0x007ab756);
                        flags = gp4::flags::merge(flags, gp4::flags::sub8(a, 2u));
                        if (a <= 2u) {
                            const uint8_t b = GP4_GLOBAL(uint8_t, 0x007a93b2);
                            flags = gp4::flags::merge(flags, gp4::flags::sub8(b, 2u));
                            if (b <= 2u) {
                                clear = true;
                            }
                        }
                    }
                }
            } else {
                const uint8_t m = GP4_GLOBAL(uint8_t, 0x007c2e7f);
                flags = gp4::flags::merge(flags, gp4::flags::sub8(m, 1u));
                if (m != 1u) {
                    clear = true;
                }
            }
        }
    }
    GP4_GLOBAL(uint8_t, 0x00831ad8) = clear ? 0u : 0xffu;
    r->eflags = flags;
}
GP4_IMPL(0x00547786, FUN_00547786, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv2_00547786")
