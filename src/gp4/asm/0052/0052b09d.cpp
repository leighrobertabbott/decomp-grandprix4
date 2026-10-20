#include <gp4/gp4.h>
#include <gp4/flags.h>

// Time subtraction on packed registers: (ch:cl:dh:dl) -= (bh:bl:ah:al), where dl is hundredths (base
// 100), dh seconds (base 60), cl minutes (base 60) and ch hours (base 24). Each field subtraction
// borrows from the next field; a borrow out of a field adds its base back. The routine ends on the
// flags of the last executed sub/add (they are the CPU's own flags for byte operations).
static inline bool sub_field(uint8_t& v, uint32_t d, uint32_t& flags) {
    flags = gp4::flags::sub8(v, d);
    const bool borrow = v < (uint8_t)d;
    v = (uint8_t)(v - d);
    return borrow;
}

static inline void add_base(uint8_t& v, uint32_t base, uint32_t& flags) {
    flags = gp4::flags::add8(v, base);
    v = (uint8_t)(v + base);
}

// FUNCTION: GP4 0x0052b09d
void __cdecl FUN_0052b09d(gp4::Regs* r) {
    const uint8_t al = (uint8_t)r->eax;
    const uint8_t ah = (uint8_t)(r->eax >> 8);
    const uint8_t bl = (uint8_t)r->ebx;
    const uint8_t bh = (uint8_t)(r->ebx >> 8);
    uint8_t cl = (uint8_t)r->ecx;
    uint8_t ch = (uint8_t)(r->ecx >> 8);
    uint8_t dl = (uint8_t)r->edx;
    uint8_t dh = (uint8_t)(r->edx >> 8);
    uint32_t f = 0u;

    // hundredths
    if (sub_field(dl, al, f)) {
        add_base(dl, 0x64u, f);
        if (sub_field(dh, 1u, f)) {
            add_base(dh, 0x3cu, f);
            if (sub_field(cl, 1u, f)) {
                add_base(cl, 0x3cu, f);
                if (sub_field(ch, 1u, f)) {
                    add_base(ch, 0x18u, f);
                }
            }
        }
    }
    // seconds
    if (sub_field(dh, ah, f)) {
        add_base(dh, 0x3cu, f);
        if (sub_field(cl, 1u, f)) {
            add_base(cl, 0x3cu, f);
            if (sub_field(ch, 1u, f)) {
                add_base(ch, 0x18u, f);
            }
        }
    }
    // minutes
    if (sub_field(cl, bl, f)) {
        add_base(cl, 0x3cu, f);
        if (sub_field(ch, 1u, f)) {
            add_base(ch, 0x18u, f);
        }
    }
    // hours
    if (sub_field(ch, bh, f)) {
        add_base(ch, 0x18u, f);
    }

    r->ecx = (r->ecx & 0xffff0000u) | ((uint32_t)ch << 8) | cl;
    r->edx = (r->edx & 0xffff0000u) | ((uint32_t)dh << 8) | dl;
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0052b09d, FUN_0052b09d, "regs(eax:u32{0,1,0x0101,0x0a05,0xff,0xffff,0x7f7f,0x3c3c,0x12345678}, ebx:u32{0,1,0x0100,0x0101,0x1700,0x0010,0x3b3b,0xffff,0x87654321}, ecx:u32{0,0,0,1,0x0100,0x0101,0x1700,0x3b3b,0xffffffff}, edx:u32{0,0,1,5,0x0101,0x0a00,0xff00,0x3c3c,0x12345678,0x80000080}) -> (ecx, edx, cf, pf, af, zf, sf, of)")
