#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags of a 32-bit SUB/CMP a - b.
static inline uint32_t subFlags(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    return (a < b ? 1u : 0u)
        | parityFlag(res)
        | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x80000000u) ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) & 0x80000000u) ? 0x800u : 0u);
}

// FILD through the scratch dword 0x006f2bf4.
static inline Ext fildScratch(uint32_t v) {
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = v;
    return gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
}

// ((a - b) sar 1) + b
static inline uint32_t midpoint(uint32_t a, uint32_t b) {
    return (uint32_t)((int32_t)(a - b) >> 1) + b;
}

// Keep the sign, raise the magnitude to at least 0x1000 (signed compares).
static inline uint32_t atLeast1000(uint32_t x) {
    if ((int32_t)x < 0) {
        uint32_t m = 0u - x;
        if ((int32_t)m < 0x1000) m = 0x1000u;
        return 0u - m;
    }
    if ((int32_t)x < 0x1000) x = 0x1000u;
    return x;
}

// |round((to - from) * [0x00627950]) + from - target| > [0x006279e0]
static inline bool farFrom(uint32_t to, uint32_t from, uint32_t target) {
    using namespace gp4::x87;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(fildScratch(to - from) * ext(GP4_GLOBAL(double, 0x00627950)));
    uint32_t d = GP4_GLOBAL(uint32_t, 0x006f2bf4) + from - target;
    if ((int32_t)d < 0) d = 0u - d;
    return (int32_t)d > GP4_GLOBAL(int32_t, 0x006279e0);
}

// FUNCTION: GP4 0x00407af6
void __cdecl FUN_00407af6(gp4::Regs* r) {
    using namespace gp4::x87;
    GP4_GLOBAL(uint32_t, 0x00627994) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00627998) = r->edx;
    gp4::call_regs(0x00407839, *r);
    if ((r->eflags & gp4::CF) != 0u) {
        GP4_GLOBAL(double, 0x00627958) = narrow(ext(GP4_GLOBAL(double, 0x006f2a54)));
        gp4::call_regs(0x00407944, *r);
        gp4::call_regs(0x00407a0d, *r);
    } else {
        const Ext start = ext(GP4_GLOBAL(double, 0x006f2a54));
        GP4_GLOBAL(double, 0x00627950) = narrow(start);
        GP4_GLOBAL(double, 0x00627958) = narrow(start);
        GP4_GLOBAL(int32_t, 0x00627990) = 0;
        GP4_GLOBAL(int32_t, 0x00627988) = 0;
        GP4_GLOBAL(int32_t, 0x0062798c) = 0;
        GP4_GLOBAL(uint32_t, 0x0062799c) = midpoint(GP4_GLOBAL(uint32_t, 0x006279b0), GP4_GLOBAL(uint32_t, 0x006279ac));
        GP4_GLOBAL(uint32_t, 0x006279a0) = midpoint(GP4_GLOBAL(uint32_t, 0x006279d0), GP4_GLOBAL(uint32_t, 0x006279cc));
        GP4_GLOBAL(uint32_t, 0x006279a4) = midpoint(GP4_GLOBAL(uint32_t, 0x006279b4), GP4_GLOBAL(uint32_t, 0x006279b8));
        GP4_GLOBAL(uint32_t, 0x006279a8) = midpoint(GP4_GLOBAL(uint32_t, 0x006279d4), GP4_GLOBAL(uint32_t, 0x006279d8));

        for (;;) {
            // length and heading of the current chord
            const uint32_t ax = GP4_GLOBAL(uint32_t, 0x006279a4) - GP4_GLOBAL(uint32_t, 0x0062799c);
            const Ext a = fildScratch(ax);
            const Ext aa = a * a;
            const uint32_t bx = GP4_GLOBAL(uint32_t, 0x006279a8) - GP4_GLOBAL(uint32_t, 0x006279a0);
            const Ext b = fildScratch(bx);
            const Ext bb = b * b;
            GP4_GLOBAL(int32_t, 0x0062797c) = fistp32(fsqrt(aa + bb));

            const Ext y = fildScratch(ax);
            const Ext x = fildScratch(bx);
            GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(fpatan(y, x) * ext(GP4_GLOBAL(double, 0x006f2be4)));
            GP4_GLOBAL(uint32_t, 0x00627978) = GP4_GLOBAL(uint32_t, 0x006f2bf4) - 0x40000000u;

            Ext s1, c1;
            fsincos(ext_i32(GP4_GLOBAL(int32_t, 0x00627978)) * ext(GP4_GLOBAL(double, 0x006f2bdc)), &s1, &c1);

            // target point rotated into the chord frame
            const uint32_t px = GP4_GLOBAL(uint32_t, 0x00627994) - GP4_GLOBAL(uint32_t, 0x0062799c);
            const uint32_t qx = GP4_GLOBAL(uint32_t, 0x00627998) - GP4_GLOBAL(uint32_t, 0x006279a0);
            const Ext pc = c1 * fildScratch(px);
            const Ext qs = s1 * fildScratch(qx);
            GP4_GLOBAL(double, 0x00627960) = narrow(pc - qs);
            const Ext qc = c1 * fildScratch(qx);
            const Ext ps = s1 * fildScratch(px);
            GP4_GLOBAL(double, 0x00627968) = narrow(qc + ps);

            const uint32_t span = GP4_GLOBAL(uint32_t, 0x00627974) - GP4_GLOBAL(uint32_t, 0x00627970);
            GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(fildScratch(span) * ext(GP4_GLOBAL(double, 0x00627950)));
            const uint32_t turn = atLeast1000(GP4_GLOBAL(uint32_t, 0x006f2bf4) + GP4_GLOBAL(uint32_t, 0x00627970)
                                              - GP4_GLOBAL(uint32_t, 0x00627978));
            Ext s2, c2;
            fsincos(fildScratch(turn) * ext(GP4_GLOBAL(double, 0x006f2bdc)), &s2, &c2);
            const Ext w = (ext_one() / c2) * ext(GP4_GLOBAL(double, 0x00627968));

            if (GP4_GLOBAL(int32_t, 0x0062797c) < 0x555) {
                GP4_GLOBAL(double, 0x00627950) = narrow(ext(GP4_GLOBAL(double, 0x006f2a54)));
            } else {
                const Ext f = (fchs(s2 * w) + ext(GP4_GLOBAL(double, 0x00627960)))
                            / ext_i32(GP4_GLOBAL(int32_t, 0x0062797c));
                GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(f * ext(GP4_GLOBAL(double, 0x006f2b14)));
                Ext t = f;
                if (GP4_GLOBAL(int32_t, 0x006f2bf4) < 0) {
                    t = ext_zero();
                    GP4_GLOBAL(int32_t, 0x00627988) += 1;
                } else if (GP4_GLOBAL(int32_t, 0x006f2bf4) >= 0x40000) {
                    t = ext_one();
                    GP4_GLOBAL(int32_t, 0x00627988) += 1;
                }
                GP4_GLOBAL(double, 0x00627950) = narrow(t);
            }

            const Ext lo = ext_i32(GP4_GLOBAL(int32_t, 0x00627980));
            const Ext denom = lo + ((fchs(lo) + ext_i32(GP4_GLOBAL(int32_t, 0x00627984)))
                                    * ext(GP4_GLOBAL(double, 0x00627950)));
            const Ext g = (w / denom) + ext(GP4_GLOBAL(double, 0x00627958));
            GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(g * ext(GP4_GLOBAL(double, 0x006f2b14)));
            Ext u = g;
            if (GP4_GLOBAL(int32_t, 0x006f2bf4) < 0) {
                u = ext_zero();
                GP4_GLOBAL(int32_t, 0x0062798c) += 1;
            } else if (GP4_GLOBAL(int32_t, 0x006f2bf4) >= 0x40000) {
                u = ext_one();
                GP4_GLOBAL(int32_t, 0x0062798c) += 1;
            }
            GP4_GLOBAL(double, 0x00627958) = narrow(u);
            GP4_GLOBAL(int32_t, 0x00627990) += 1;
            gp4::call_regs(0x00407944, *r);

            if (GP4_GLOBAL(int32_t, 0x00627990) >= 6 || GP4_GLOBAL(int32_t, 0x00627988) >= 2
                || GP4_GLOBAL(int32_t, 0x0062798c) >= 2) {
                break;
            }
            if (farFrom(GP4_GLOBAL(uint32_t, 0x006279a4), GP4_GLOBAL(uint32_t, 0x0062799c), GP4_GLOBAL(uint32_t, 0x00627994))) {
                continue;
            }
            if (farFrom(GP4_GLOBAL(uint32_t, 0x006279a8), GP4_GLOBAL(uint32_t, 0x006279a0), GP4_GLOBAL(uint32_t, 0x00627998))) {
                continue;
            }
            break;
        }
    }

    const uint32_t ax = GP4_GLOBAL(uint32_t, 0x006279a4) - GP4_GLOBAL(uint32_t, 0x0062799c);
    const Ext a = fildScratch(ax);
    const Ext aa = a * a;
    const uint32_t a8 = GP4_GLOBAL(uint32_t, 0x006279a8);
    const uint32_t a0 = GP4_GLOBAL(uint32_t, 0x006279a0);
    const uint32_t bx = a8 - a0;
    const Ext b = fildScratch(bx);
    const Ext bb = b * b;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(fsqrt(aa + bb));

    r->eax = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    r->edx = bx;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(a8, a0);
}
GP4_IMPL(0x00407af6, FUN_00407af6, "regs(eax:i32[-0x1000000..0x1000000], edx:i32[-0x1000000..0x1000000]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); cases=800; globals=layout=state_00407af6+quad_00407af6")
