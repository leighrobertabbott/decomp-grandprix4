#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal ... popal; clc/stc key handler. Polls bit codes 0x70..0x77 and [0x852dec] through the
// bitmap testers 0x510e04 (wanted bit set -> ZF) and 0x510e2d (newly set -> ZF), toggling the
// byte state at 0x852d9b..0x852da4 and calling 0x518890 / 0x5188a6 around it. Every GPR is
// restored by popal; the flags are those of the last flag writer with CF replaced by the
// clc (0x51d793) or stc (0x51d796) epilogue.
// FUNCTION: GP4 0x0051d607
static inline void set_cx(gp4::Regs& c, uint32_t v) { c.ecx = (c.ecx & 0xffff0000u) | v; }
static inline void set_cl(gp4::Regs& c, uint32_t v) { c.ecx = (c.ecx & 0xffffff00u) | (v & 0xffu); }

void __cdecl FUN_0051d607(gp4::Regs* r) {
    uint8_t& b9b = GP4_GLOBAL(uint8_t, 0x00852d9b);
    uint8_t& b9c = GP4_GLOBAL(uint8_t, 0x00852d9c);
    uint8_t& b9d = GP4_GLOBAL(uint8_t, 0x00852d9d);
    uint8_t& b9e = GP4_GLOBAL(uint8_t, 0x00852d9e);
    uint8_t& b9f = GP4_GLOBAL(uint8_t, 0x00852d9f);
    uint8_t& ba0 = GP4_GLOBAL(uint8_t, 0x00852da0);
    uint8_t& ba1 = GP4_GLOBAL(uint8_t, 0x00852da1);
    uint8_t& ba2 = GP4_GLOBAL(uint8_t, 0x00852da2);
    uint8_t& ba3 = GP4_GLOBAL(uint8_t, 0x00852da3);
    uint8_t& ba4 = GP4_GLOBAL(uint8_t, 0x00852da4);

    gp4::Regs c = *r;
    uint32_t fl = r->eflags;
    uint32_t carry = 1u;

    ba2 = 0;
    set_cx(c, 0x72);
    gp4::call_regs(0x00510e2d, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        ba2 = 0xff;
        ba4 = 0;
        goto done;
    }
    set_cx(c, 0x72);
    gp4::call_regs(0x00510e04, c);
    fl = c.eflags;
    if (!(fl & gp4::flags::ZF)) {
        set_cl(c, GP4_GLOBAL(uint8_t, 0x00852dec));
        gp4::call_regs(0x00510e04, c);
        fl = c.eflags;
        if (!(fl & gp4::flags::ZF)) goto keys;
    }
    ba1 = 0xff;
    goto done;

keys:
    ba1 = 0;
    fl = gp4::flags::merge(fl, gp4::flags::logic8(b9d));
    if (b9d != 0) goto done;
    set_cx(c, 0x77);
    gp4::call_regs(0x00510e04, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        gp4::call_regs(0x00518890, c);
        fl = c.eflags;
        b9d = 0x0c;
        goto done;
    }
    fl = gp4::flags::merge(fl, gp4::flags::logic8(b9e));
    if (b9e != 0) goto done;
    set_cx(c, 0x76);
    gp4::call_regs(0x00510e04, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        gp4::call_regs(0x00518890, c);
        fl = c.eflags;
        b9e = 3;
        goto done;
    }
    set_cx(c, 0x73);
    gp4::call_regs(0x00510e04, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        gp4::call_regs(0x00518890, c);
        fl = c.eflags;
        ba3 = 1;
        goto done;
    }
    fl = gp4::flags::merge(fl, gp4::flags::logic8(ba3));
    if (ba3 != 0) {
        gp4::call_regs(0x005188a6, c);
        fl = c.eflags;
        ba3 = 0;
    }
    set_cx(c, 0x75);
    gp4::call_regs(0x00510e04, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        gp4::call_regs(0x00518890, c);
        fl = c.eflags;
        ba0 = 1;
        goto done;
    }
    fl = gp4::flags::merge(fl, gp4::flags::logic8(ba0));
    if (ba0 != 0) {
        gp4::call_regs(0x005188a6, c);
        fl = c.eflags;
        ba0 = 0;
    }
    set_cx(c, 0x74);
    gp4::call_regs(0x00510e04, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        gp4::call_regs(0x00518890, c);
        fl = c.eflags;
        b9f = 1;
        goto done;
    }
    fl = gp4::flags::merge(fl, gp4::flags::logic8(b9f));
    if (b9f != 0) {
        gp4::call_regs(0x005188a6, c);
        fl = c.eflags;
        b9f = 0;
    }

    fl = gp4::flags::merge(fl, gp4::flags::sub8(b9c, 3));
    if (b9c != 3) {
        set_cx(c, 0x70);
        gp4::call_regs(0x00510e2d, c);
        fl = c.eflags;
        if (fl & gp4::flags::ZF) {
            fl = gp4::flags::merge(fl, gp4::flags::sub8(b9c, 2));
            if (b9c != 2) {
                b9c = 1;
                goto done;
            }
        } else {
            b9c = 0;
        }
    }
    set_cx(c, 0x71);
    gp4::call_regs(0x00510e2d, c);
    fl = c.eflags;
    if (fl & gp4::flags::ZF) {
        fl = gp4::flags::merge(fl, gp4::flags::sub8(b9b, 2));
        if (b9b != 2) {
            b9b = 1;
            goto done;
        }
    } else {
        b9b = 0;
    }
    ba2 = 0xff;
    carry = 0u;

done:
    r->eflags = gp4::flags::merge(r->eflags, (fl & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0051d607, FUN_0051d607, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=keys_0051d607")
