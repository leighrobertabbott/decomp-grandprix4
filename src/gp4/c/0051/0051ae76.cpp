#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clamps [0x007ad818] to 0xb0..0x100 and returns the flags of the final `cmp [0x7ad818], 0x100`.
static inline uint32_t Clamp7ad818(int32_t value) {
    GP4_GLOBAL(int32_t, 0x007ad818) = value;
    if (GP4_GLOBAL(int32_t, 0x007ad818) < 0xb0) {
        GP4_GLOBAL(int32_t, 0x007ad818) = 0xb0;
    }
    const int32_t low = GP4_GLOBAL(int32_t, 0x007ad818);
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(low), 0x100u);
    if (low > 0x100) {
        GP4_GLOBAL(int32_t, 0x007ad818) = 0x100;
    }
    return flags;
}

// Key-driven adjust of [0x007ad818] (kept in 0xb0..0x100): only when bytes [0x0066d7a4] and
// [0x00852d6a] are zero, the viewed car [0x00679d2c] is the player's [0x00679d30], [0x0095416a] == 2,
// [0x007ad81d] == 0xff and [0x00645950] <= 2 (for 2 also [0x00a53ad0] == 0 or [0x00645954] == 0).
// Key 0xbb held in the key bitmap (0x00510e04) steps it down by 4, else key 0xbd steps it up by 4.
// eax = [0x00679d30] after the car compare; ecx is the 0x00510e04 result.
// FUNCTION: GP4 0x0051ae76
void __cdecl FUN_0051ae76(gp4::Regs* r) {
    uint32_t flags;
    const uint8_t a = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (a != 0u) {
        flags = gp4::flags::logic8(a);
    } else {
        const uint8_t b = GP4_GLOBAL(uint8_t, 0x00852d6a);
        if (b != 0u) {
            flags = gp4::flags::logic8(b);
        } else {
            const uint32_t player = GP4_GLOBAL(uint32_t, 0x00679d30);
            r->eax = player;
            const uint32_t viewed = GP4_GLOBAL(uint32_t, 0x00679d2c);
            flags = gp4::flags::sub32(player, viewed);
            if (player == viewed) {
                const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0095416a);
                flags = gp4::flags::sub8(mode, 2u);
                if (mode == 2u) {
                    const uint8_t view = GP4_GLOBAL(uint8_t, 0x007ad81d);
                    flags = gp4::flags::sub8(view, 0xffu);
                    if (view == 0xffu) {
                        const int32_t level = GP4_GLOBAL(int32_t, 0x00645950);
                        flags = gp4::flags::sub32(static_cast<uint32_t>(level), 2u);
                        bool run = level <= 2;
                        if (run && level == 2 && GP4_GLOBAL(uint32_t, 0x00a53ad0) != 0u) {
                            const uint32_t other = GP4_GLOBAL(uint32_t, 0x00645954);
                            flags = gp4::flags::logic32(other);
                            run = other == 0u;
                        }
                        if (run) {
                            r->ecx = 0xbbu;
                            gp4::call_regs(0x00510e04, *r);
                            if ((r->eflags & gp4::ZF) != 0u) {
                                flags = Clamp7ad818(GP4_GLOBAL(int32_t, 0x007ad818) - 4);
                            } else {
                                r->ecx = 0xbdu;
                                gp4::call_regs(0x00510e04, *r);
                                if ((r->eflags & gp4::ZF) == 0u) {
                                    return;
                                }
                                flags = Clamp7ad818(GP4_GLOBAL(int32_t, 0x007ad818) + 4);
                            }
                        }
                    }
                }
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051ae76, FUN_0051ae76, "regs() -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_008245a0+gate_0051ae76")
