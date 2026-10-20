#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// Hand-written routine: fills the 0x160-byte record at edi from the build state at 0x00870580..
// 0x00870650 / 0x00874b08.. / 0x00874dd8.. (and, when byte 0x00852c38 is set, from the overrides
// at 0x00852d0c..0x00852d18 and the four countdown words 0x00852c84..0x00852c8a). Calls the
// original helpers 0x00553d5f (split 0x00870640..48 into the record), 0x0055a53e, 0x00559bc0,
// 0x0040ad1a and 0x0040b629 with the registers they see in the original. +0x150 = fistp of
// tan(i16 [0x00870610] * [0x006f2bc4]) * [0x006f2b5c], all on the x87 stack.
// Register effects: eax = the last word written (zero-extended), ebx/ecx as left by 0x0055a53e,
// edx = the dword whose low word went to +0x132; the flags are those of the last compare/dec/neg.

static uint32_t countdown(uint16_t* counter, uint32_t flags, bool* fired) {
    const uint16_t w = *counter;
    flags = gp4::flags::sub16(w, 0xffffu);
    *fired = false;
    if (static_cast<int16_t>(w) > -1) {
        flags = gp4::flags::dec16(w, flags);
        const uint16_t d = static_cast<uint16_t>(w - 1u);
        *counter = d;
        *fired = static_cast<int16_t>(d) < 0;
    }
    return flags;
}

// FUNCTION: GP4 0x0055a082
void __cdecl FUN_0055a082(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs c = {};
    c.ebx = r->ebx;
    c.ecx = r->ecx;
    c.edx = r->edx;
    c.esi = r->esi;
    c.edi = r->edi;
    c.ebp = r->ebp;
    c.eflags = r->eflags;

    uint32_t v = GP4_GLOBAL(uint32_t, 0x00870628);
    if (GP4_GLOBAL(uint8_t, 0x00852c38) != 0) {
        v = GP4_GLOBAL(uint32_t, 0x00852d18);
        GP4_GLOBAL(uint32_t, 0x00870628) = v;
    }
    GP4_FIELD(uint32_t, rec, 0xec) = v;
    GP4_FIELD(uint16_t, rec, 0x0a) = static_cast<uint16_t>(static_cast<int32_t>(v) >> 16);
    v = GP4_GLOBAL(uint32_t, 0x00870638);
    GP4_FIELD(uint32_t, rec, 0xe4) = v;
    GP4_FIELD(uint16_t, rec, 0x00) = static_cast<uint16_t>(static_cast<int32_t>(v) >> 16);
    v = GP4_GLOBAL(uint32_t, 0x0087063c);
    GP4_FIELD(uint32_t, rec, 0xe8) = v;
    GP4_FIELD(uint16_t, rec, 0x02) = static_cast<uint16_t>(static_cast<int32_t>(v) >> 16);
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(v) >> 16);
    gp4::call_regs(0x00553d5f, c);

    GP4_FIELD(uint16_t, rec, 0x130) = 0x4000;
    GP4_FIELD(uint32_t, rec, 0x30) = GP4_GLOBAL(uint32_t, 0x00874b40);
    GP4_FIELD(uint32_t, rec, 0x38) = GP4_GLOBAL(uint32_t, 0x00874b44) & 0xffffff3fu;
    GP4_FIELD(uint32_t, rec, 0x34) = GP4_GLOBAL(uint32_t, 0x00874b48) & 0xffffff3fu;
    GP4_FIELD(uint32_t, rec, 0x2c) = (GP4_GLOBAL(uint8_t, 0x008705be) & 0x30u) | GP4_GLOBAL(uint32_t, 0x008705f0);
    rec[0x12] = 0;
    rec[0x10] = 0;
    rec[0x0e] = static_cast<uint8_t>(rec[0x0e] | GP4_GLOBAL(uint8_t, 0x0086857e) | GP4_GLOBAL(uint8_t, 0x00868580));
    GP4_FIELD(uint16_t, rec, 0x16) = 0;
    GP4_FIELD(uint16_t, rec, 0x18) = 0;
    uint16_t w1a = GP4_GLOBAL(uint16_t, 0x008705e8);
    if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0) {
        w1a = static_cast<uint16_t>((w1a + GP4_GLOBAL(uint16_t, 0x00868570)) | 0x2000u);
    }
    GP4_FIELD(uint16_t, rec, 0x1a) = w1a;
    rec[0x1c] = GP4_GLOBAL(uint8_t, 0x008705fe);
    rec[0x1d] = GP4_GLOBAL(uint8_t, 0x00870600);
    GP4_FIELD(uint16_t, rec, 0x1e) = 0;
    rec[0x66] = 0;
    rec[0x92] = 0;
    rec[0x20] = GP4_GLOBAL(uint8_t, 0x0086857a) != 0 ? GP4_GLOBAL(uint8_t, 0x0086856e) : 0x3c;
    GP4_FIELD(uint16_t, rec, 0x48) = static_cast<uint16_t>((GP4_GLOBAL(uint16_t, 0x008705bc) & 0xff00u) | GP4_GLOBAL(uint8_t, 0x008705be));
    GP4_FIELD(uint32_t, rec, 0x40) = 0;
    rec[0x25] = GP4_GLOBAL(uint8_t, 0x0087061b);
    rec[0x24] = GP4_GLOBAL(uint8_t, 0x0087061a);
    rec[0x23] = GP4_GLOBAL(uint8_t, 0x00870619);
    rec[0x22] = GP4_GLOBAL(uint8_t, 0x00870618);
    const uint32_t b44 = GP4_GLOBAL(uint32_t, 0x00874b44);
    const uint32_t keep = GP4_GLOBAL(uint32_t, 0x00870634);
    GP4_GLOBAL(uint32_t, 0x00870634) = 0;
    const uint32_t mask = ~(3u & ~keep);
    const uint32_t bits = (GP4_FIELD(uint32_t, rec, 0x30) | GP4_FIELD(uint32_t, rec, 0x34)) & 0x30u;
    c.eax = (b44 & mask) | bits;
    c.ecx = bits;
    c.edx = mask;
    GP4_FIELD(uint32_t, rec, 0x3c) = c.eax;

    if (GP4_GLOBAL(uint8_t, 0x008705c0) == 0) {
        if (GP4_GLOBAL(uint8_t, 0x00852c38) != 0) {
            GP4_GLOBAL(uint32_t, 0x00870640) = GP4_GLOBAL(uint32_t, 0x00852d0c);
            GP4_GLOBAL(uint32_t, 0x00870644) = GP4_GLOBAL(uint32_t, 0x00852d10);
            GP4_GLOBAL(uint32_t, 0x00870648) = GP4_GLOBAL(uint32_t, 0x00852d14);
            gp4::call_regs(0x00553d5f, c);
        }
        const uint8_t m = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008705c4) | 0xcfu);
        c.eax = (c.eax & 0xffffff00u) | m;
        rec[0x48] = static_cast<uint8_t>(rec[0x48] & m);
        if ((GP4_GLOBAL(uint8_t, 0x008705b7) & 0x40u) != 0) {
            GP4_FIELD(uint32_t, rec, 0x40) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x38) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x34) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x30) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x3c) |= 3u;
            c.eax = 3u;
        }
        if (GP4_GLOBAL(uint32_t, 0x00874b08) == 0) {
            GP4_FIELD(uint32_t, rec, 0x40) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x38) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x34) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x30) |= 0x703u;
            GP4_FIELD(uint32_t, rec, 0x3c) |= 0x703u;
            GP4_GLOBAL(uint32_t, 0x00874b08) = GP4_GLOBAL(uint32_t, 0x00874b0c);
            c.eax = 0x703u;
        }
        uint16_t ax = GP4_GLOBAL(uint16_t, 0x008705c2);
        if ((ax >> 8) != 0) {
            ax = static_cast<uint16_t>(ax ^ 0xff00u);
            rec[0x38] = static_cast<uint8_t>(rec[0x38] & (ax >> 8));
        }
        c.eax = (c.eax & 0xffff0000u) | ax;
    }

    c.eax = (c.eax & 0xffff0000u) | (GP4_FIELD(uint16_t, rec, 0x1a) & 0x7ffu);
    rec[0x28] = GP4_GLOBAL(uint8_t, 0x008705af);
    const uint8_t b1 = GP4_GLOBAL(uint8_t, 0x008705b1);
    rec[0x29] = b1;
    c.eax = (c.eax & 0xffffff00u) | b1;
    gp4::call_regs(0x0055a53e, c);

    if ((rec[0x2c] & 0x0cu) == 0) {
        rec[0x48] = static_cast<uint8_t>(rec[0x48] & 0xfbu);
    }
    GP4_FIELD(uint16_t, rec, 0xa6) = 0x7700;
    GP4_FIELD(uint16_t, rec, 0xa4) = 0x7700;
    const int16_t angle = GP4_GLOBAL(int16_t, 0x00870610);
    GP4_FIELD(int16_t, rec, 0x26) = angle;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = angle;
    {
        using namespace gp4::x87;
        const Ext x = ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * ext(GP4_GLOBAL(double, 0x006f2bc4));
        Ext s, co;
        fsincos(x, &s, &co);
        const Ext t = (s / co) * ext(GP4_GLOBAL(double, 0x006f2b5c));
        GP4_FIELD(int32_t, rec, 0x150) = fistp32(t);
    }
    GP4_FIELD(uint32_t, rec, 0x44) = 0;
    if (GP4_GLOBAL(uint8_t, 0x008705c0) == 0) {
        c.eax = GP4_GLOBAL(uint8_t, 0x008705c5);
        gp4::call_regs(0x00559bc0, c);
    }
    rec[0x2b] = 0xff;
    if (GP4_GLOBAL(uint32_t, 0x00874b08) != 0) {
        GP4_GLOBAL(uint32_t, 0x00874b08) -= 1;
    }
    c.eax = GP4_GLOBAL(uint32_t, 0x008705cc);
    gp4::call_regs(0x0040ad1a, c);
    GP4_FIELD(uint16_t, rec, 0x7e) = GP4_GLOBAL(uint16_t, 0x00874dd8);
    GP4_FIELD(uint16_t, rec, 0x80) = GP4_GLOBAL(uint16_t, 0x00874ddc);
    rec[0x83] = static_cast<uint8_t>(GP4_GLOBAL(int32_t, 0x00874df0) >> 3);
    rec[0x86] = static_cast<uint8_t>(GP4_GLOBAL(int32_t, 0x00874df4) >> 3);
    rec[0x84] = static_cast<uint8_t>(GP4_GLOBAL(int32_t, 0x00874df8) >> 1);
    rec[0x87] = static_cast<uint8_t>(GP4_GLOBAL(int32_t, 0x00874dfc) >> 1);
    GP4_FIELD(uint16_t, rec, 0x88) = 0xffff;
    if (GP4_GLOBAL(int32_t, 0x00645950) == 1) {
        GP4_FIELD(uint16_t, rec, 0x9c) = 0xffff;
    } else {
        GP4_FIELD(uint16_t, rec, 0x138) = 0xffff;
    }
    if (GP4_GLOBAL(int32_t, 0x00645950) == 1) {
        GP4_FIELD(uint16_t, rec, 0x8a) = 0xffff;
    } else {
        GP4_FIELD(uint16_t, rec, 0x13e) = 0xffff;
    }
    const uint32_t serial = GP4_GLOBAL(uint32_t, 0x0073d208);
    GP4_FIELD(uint16_t, rec, 0x7a) = static_cast<uint16_t>(serial);
    GP4_GLOBAL(uint32_t, 0x0073d208) = serial + 1u;
    c.eax = serial;
    gp4::call_regs(0x0040b629, c);

    uint32_t lo;
    uint32_t hi;
    if (GP4_GLOBAL(uint8_t, 0x00852c38) == 0 && (rec[0x48] & 4u) == 0) {
        lo = GP4_GLOBAL(uint32_t, 0x0087059c);
        hi = GP4_GLOBAL(uint32_t, 0x008705a0);
    } else {
        lo = GP4_GLOBAL(uint32_t, 0x008705a4);
        hi = GP4_GLOBAL(uint32_t, 0x008705a8);
    }
    GP4_FIELD(uint16_t, rec, 0xaa) = static_cast<uint16_t>(lo);
    GP4_FIELD(uint16_t, rec, 0x132) = static_cast<uint16_t>(hi);
    const uint16_t* const table = GP4_ARRAY(uint16_t, 0x006f3d94);
    GP4_FIELD(uint16_t, rec, 0xbc) = table[GP4_GLOBAL(uint8_t, 0x0086857e) & 7u];
    uint16_t ax = table[(GP4_GLOBAL(uint8_t, 0x00868580) >> 3) & 7u];
    GP4_FIELD(uint16_t, rec, 0xbe) = ax;
    GP4_FIELD(uint16_t, rec, 0xc4) = 0x1e;
    GP4_FIELD(uint16_t, rec, 0xc6) = 0x1d;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00852c38);
    uint32_t flags = gp4::flags::sub8(mode, 0u);
    if (mode != 0) {
        bool fired;
        flags = countdown(&GP4_GLOBAL(uint16_t, 0x00852c84), flags, &fired);
        if (fired) {
            ax = GP4_GLOBAL(uint16_t, 0x00852c7c);
            GP4_FIELD(uint16_t, rec, 0xfc) = ax;
        }
        flags = countdown(&GP4_GLOBAL(uint16_t, 0x00852c86), flags, &fired);
        if (fired) {
            const uint16_t a = static_cast<uint16_t>(0x4000u - GP4_GLOBAL(uint16_t, 0x00852c7e));
            flags = gp4::flags::neg16(a);
            ax = static_cast<uint16_t>(0u - a);
            GP4_FIELD(uint16_t, rec, 0xfc) = ax;
        }
        flags = countdown(&GP4_GLOBAL(uint16_t, 0x00852c88), flags, &fired);
        if (fired) {
            ax = GP4_GLOBAL(uint16_t, 0x00852c80);
            GP4_FIELD(uint16_t, rec, 0xfe) = ax;
        }
        flags = countdown(&GP4_GLOBAL(uint16_t, 0x00852c8a), flags, &fired);
        if (fired) {
            const uint16_t a = static_cast<uint16_t>(0x4000u - GP4_GLOBAL(uint16_t, 0x00852c82));
            flags = gp4::flags::neg16(a);
            ax = static_cast<uint16_t>(0u - a);
            GP4_FIELD(uint16_t, rec, 0xfe) = ax;
        }
    }

    r->eax = ax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = hi;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055a082, FUN_0055a082, "regs(edi:ptr[0x160]:bytes, ebx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mask_0055a082")
