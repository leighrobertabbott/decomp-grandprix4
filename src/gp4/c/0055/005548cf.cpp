#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clamps the signed byte cl into the closed range 0..15 after pushing a value that lies inside the
// signed byte interval [lo, hi] = [[0x874edc], [0x874edd]] out of it: cl inside the interval moves to
// whichever edge is nearer (a tie goes to hi + 1), i.e. lo - 1 or hi + 1; cl outside is kept. Then
// cl = min(cl, 15) (signed), the flags are those of `cmp cl, 0`, and a negative cl becomes 0.
// eax is saved/restored; only cl of ecx changes.
// FUNCTION: GP4 0x005548cf
void __cdecl FUN_005548cf(gp4::Regs* r) {
    int8_t cl = static_cast<int8_t>(r->ecx);
    const int8_t lo = GP4_GLOBAL(int8_t, 0x00874edc);
    if (!(cl < lo)) {
        const int8_t hi = GP4_GLOBAL(int8_t, 0x00874edd);
        if (!(cl > hi)) {
            int8_t below = static_cast<int8_t>(static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00874edc)) - static_cast<uint8_t>(cl));
            if (below < 0) {
                below = static_cast<int8_t>(0 - static_cast<uint8_t>(below));
            }
            int8_t above = static_cast<int8_t>(static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00874edd)) - static_cast<uint8_t>(cl));
            if (above < 0) {
                above = static_cast<int8_t>(0 - static_cast<uint8_t>(above));
            }
            if (below < above) {
                cl = static_cast<int8_t>(static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00874edc)) - 1u);
            } else {
                cl = static_cast<int8_t>(static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00874edd)) + 1u);
            }
        }
    }
    if (cl > 0xf) {
        cl = 0xf;
    }
    const uint32_t arith = gp4::flags::sub8(static_cast<uint8_t>(cl), 0u);
    if (cl < 0) {
        cl = 0;
    }
    r->ecx = (r->ecx & 0xffffff00u) | static_cast<uint8_t>(cl);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005548cf, FUN_005548cf, "regs(ecx:u32) -> (ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
