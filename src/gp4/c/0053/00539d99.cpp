#include <gp4/gp4.h>
#include <gp4/flags.h>

// One edge of the view rectangle in screen units: ((pos - origin) * scale) truncated to 32 bits, times the
// signed zoom, divided (idiv) by the range when that is nonzero (else 0), >> 14 (sar), plus the bias.
static inline int32_t viewEdge(int32_t pos, int32_t origin, uint32_t scale, int32_t zoom, int32_t range,
                               int32_t bias) {
    int32_t quotient = 0;
    if (range != 0) {
        const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(pos - origin) * scale);
        int32_t rem;
        quotient = gp4::idiv64(static_cast<int64_t>(scaled) * static_cast<int64_t>(zoom), range, &rem);
    }
    return (quotient >> 14) + bias;
}

// pushal/popal wrapper. Mode byte [0x0082fcf0] (values above 3 count as 3) indexes the 4-entry jump
// table 0x00539db8: modes 0 and 3 go straight to popal/ret, modes 1 and 2 map the two corner pairs
// ([0x82fcd0],[0x82fcd8] against 0x8310c0/c4/zoom 0x83137b and [0x82fcdc],[0x82fcd4] against
// 0x8310cc/d0/zoom 0x83137f) to screen x/y (ebx, ebp, ecx, eax), order them (0x00539d8d), clamp them to
// the rectangle at esi (0x00539d4a) and, when the clamped x pair and y pair both differ, queue the
// rectangle with 0x00549db9, then 0x00531af4 stores 0x4a52 and the empty 0x00531b30 runs with
// edx = [0x0083132f]. popal restores every register; only the flags of the last flag writer remain.
// FUNCTION: GP4 0x00539d99
void __cdecl FUN_00539d99(gp4::Regs* r) {
    uint32_t mode = GP4_GLOBAL(uint8_t, 0x0082fcf0);
    if (mode >= 4u) {
        mode = 3u;
    }
    const uint32_t slot = mode << 2;
    if (mode != 1u && mode != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(slot, 0x00539db8u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.ebx = static_cast<uint32_t>(viewEdge(GP4_GLOBAL(int32_t, 0x0082fcd0), GP4_GLOBAL(int32_t, 0x008310c0),
                                           0x278u, GP4_GLOBAL(int32_t, 0x0083137b),
                                           GP4_GLOBAL(int32_t, 0x008310c4), 0x64));
    c.ebp = static_cast<uint32_t>(viewEdge(GP4_GLOBAL(int32_t, 0x0082fcd8), GP4_GLOBAL(int32_t, 0x008310c0),
                                           0x278u, GP4_GLOBAL(int32_t, 0x0083137b),
                                           GP4_GLOBAL(int32_t, 0x008310c4), 0x64));
    c.ecx = static_cast<uint32_t>(viewEdge(GP4_GLOBAL(int32_t, 0x0082fcdc), GP4_GLOBAL(int32_t, 0x008310cc),
                                           0x13cu, GP4_GLOBAL(int32_t, 0x0083137f),
                                           GP4_GLOBAL(int32_t, 0x008310d0), 0x60));
    c.eax = static_cast<uint32_t>(viewEdge(GP4_GLOBAL(int32_t, 0x0082fcd4), GP4_GLOBAL(int32_t, 0x008310cc),
                                           0x13cu, GP4_GLOBAL(int32_t, 0x0083137f),
                                           GP4_GLOBAL(int32_t, 0x008310d0), 0x60));
    gp4::call_regs(0x00539d8d, c);
    gp4::call_regs(0x00539d4a, c);

    uint32_t flags = gp4::flags::sub32(c.ebx, c.ebp);
    if (c.ebx != c.ebp) {
        flags = gp4::flags::sub32(c.eax, c.ecx);
        if (c.eax != c.ecx) {
            gp4::Regs q = c;
            gp4::call_regs(0x00549db9, q);

            gp4::Regs tag = c;
            tag.eax = 0x4a52u;
            gp4::call_regs(0x00531af4, tag);

            gp4::Regs tail = c;
            tail.edx = GP4_GLOBAL(uint32_t, 0x0083132f);
            tail.eflags = tag.eflags;
            gp4::call_regs(0x00531b30, tail);
            flags = tail.eflags;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539d99, FUN_00539d99, "regs(esi:ptr[0x40]:u16[0..0x1800]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bounds_00542c43+grid_00542aa8+list_00549db9+scale_00539d99")
