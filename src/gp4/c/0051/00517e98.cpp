#include <gp4/gp4.h>
#include <gp4/flags.h>

// Character tail used for each digit: 0x2d ('-') becomes 0x15 and '0'..'9' are shifted down by 0x1a.
// `arith` receives the flags of the last compare/sub executed on that path.
static inline uint8_t MapDigitChar(uint8_t c, uint32_t* arith) {
    uint32_t f = gp4::flags::sub8(c, 0x2du);
    if (c == 0x2du) {
        c = 0x15;
    }
    f = gp4::flags::sub8(c, 0x30u);
    if (c >= 0x30u) {
        f = gp4::flags::sub8(c, 0x39u);
        if (c <= 0x39u) {
            f = gp4::flags::sub8(c, 0x1au);
            c = (uint8_t)(c - 0x1au);
        }
    }
    *arith = f;
    return c;
}

// With byte [0x66d7a4] clear and byte [0x7c2ea9] negative: claims bitmap bit 4 of 0x852d64 (0x51d187),
// resets the text cursor bytes 0x88fd94..97, sets the colour bytes (0x51d48e 0x4f, 0x51d494 0), writes the
// 5-byte header at 0x89152c (0x516927), adds [0x679b18] to the total [0x66d854] unless [0x66d848] is set,
// copies it to 0xa4b328 and prints its packed-BCD digits (0x51fd0c) as "dd.d" (0x517318 for the two high
// digits, then '.', then the hundreds digit), then 0x516940 and 0x5142b4; 0x88fd9d is 1 during and 0x80
// after. Otherwise the dwords 0x66d854 and 0x66d848 are cleared.
// FUNCTION: GP4 0x00517e98
void __cdecl FUN_00517e98(gp4::Regs* r) {
    const uint8_t master = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    uint32_t fl = gp4::flags::logic8(master);
    if (master == 0u) {
        const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        fl = gp4::flags::logic8(mode);
        if ((int8_t)mode < 0) {
            GP4_GLOBAL(uint8_t, 0x0088fd9d) = 1;
            r->eax = 4;
            GP4_GLOBAL(uint32_t, 0x00852d64) &= ~0x10u;
            gp4::call_regs(0x0051d187, *r);
            GP4_GLOBAL(uint8_t, 0x0088fd94) = 0x14;
            GP4_GLOBAL(uint8_t, 0x0088fd95) = 1;
            GP4_GLOBAL(uint8_t, 0x0088fd96) = 0;
            GP4_GLOBAL(uint8_t, 0x0088fd97) = 0;
            r->eax = (r->eax & 0xffffff00u) | 0x4fu;
            gp4::call_regs(0x0051d48e, *r);
            r->eax &= 0xffffff00u;
            gp4::call_regs(0x0051d494, *r);
            gp4::call_regs(0x00516927, *r);

            uint32_t total = GP4_GLOBAL(uint32_t, 0x0066d854);
            if (GP4_GLOBAL(uint32_t, 0x0066d848) == 0u) {
                total += GP4_GLOBAL(uint32_t, 0x00679b18);
                GP4_GLOBAL(uint32_t, 0x0066d854) = total;
            }
            GP4_GLOBAL(uint32_t, 0x00a4b328) = total;
            r->eax = total;
            gp4::call_regs(0x0051fd0c, *r);

            const uint32_t saved = r->eax >> 8;
            r->eax = (saved & 0xffff0000u) | ((saved & 0xffffu) >> 4);
            gp4::call_regs(0x00517318, *r);

            uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
            uint32_t arith = 0;
            out[0] = MapDigitChar(0x2e, &arith);
            const uint8_t last = MapDigitChar((uint8_t)((saved & 0xfu) + 0x30u), &arith);
            out[1] = last;
            r->edi += 2;
            r->eax = (saved & 0xffffff00u) | last;

            gp4::cdecl_<0x00516940, void>();
            gp4::call_regs(0x005142b4, *r);
            GP4_GLOBAL(uint8_t, 0x0088fd9d) = 0x80;
            r->eflags = gp4::flags::merge(r->eflags, arith);
            return;
        }
    }
    GP4_GLOBAL(uint32_t, 0x0066d854) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d848) = 0;
    r->eflags = gp4::flags::merge(r->eflags, fl);
}
GP4_IMPL(0x00517e98, FUN_00517e98, "regs() -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c")
