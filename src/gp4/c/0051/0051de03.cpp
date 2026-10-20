#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051de03
// Register routine (esi = record from 0x0066d820). Selector byte 0x007ab54a:
//   0            -> record+0x6f |= (~byte 0x00679d1b) & 0x0c
//   axis codes   -> scale the dword at 0x007ab158 between 0x007ab13c and the limit
//                   0x007ab138 (when its offset has the opposite sign to 0x007ab134's)
//                   or 0x007ab134, to +-0x3f80/4 (Q14 >> 2), store the word at +0x6a and
//                   set bit 8 / bit 4 of +0x6f beyond +-0x800.
//   other        -> nothing.
// bl keeps the selector byte; eax/ecx/edx keep the division's quotient/divisor/remainder.

static inline int32_t clamp_3f80(int32_t q) {
    uint32_t u = static_cast<uint32_t>(q);
    if (q < 0) {
        u = 0u - u;
        if (u >= 0x3f80u) u = 0x3f80u;
        u = 0u - u;
    } else {
        if (u >= 0x3f80u) u = 0x3f80u;
    }
    return static_cast<int32_t>(u);
}

void __cdecl FUN_0051de03(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t sel = GP4_GLOBAL(uint8_t, 0x007ab54a);
    r->ebx = (r->ebx & ~0xffu) | sel;

    if (sel == 0) {
        const uint8_t al = static_cast<uint8_t>((GP4_GLOBAL(uint8_t, 0x00679d1b) ^ 0xffu) & 0x0cu);
        r->eax = (r->eax & ~0xffu) | al;
        const uint8_t b = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x6f) | al);
        GP4_FIELD(uint8_t, rec, 0x6f) = b;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(b));
        return;
    }

    const bool axis = sel == 6 || sel == 7 || sel == 0x20 || sel == 0x22 || sel == 0x23 ||
                      sel == 0x24 || sel == 8 || sel == 9 || sel == 0x21 || sel == 0x25 ||
                      sel == 0x26 || sel == 0x27;
    if (!axis) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(sel, 0x27));
        return;
    }

    const int32_t centre = GP4_GLOBAL(int32_t, 0x007ab13c);
    const int32_t pos = GP4_GLOBAL(int32_t, 0x007ab158);
    const int32_t lim134 = GP4_GLOBAL(int32_t, 0x007ab134);
    const int32_t lim138 = GP4_GLOBAL(int32_t, 0x007ab138);

    const uint32_t off134 = static_cast<uint32_t>(lim134) - static_cast<uint32_t>(centre);
    const uint32_t offpos = static_cast<uint32_t>(pos) - static_cast<uint32_t>(centre);
    const bool use138 = static_cast<int32_t>(offpos ^ off134) < 0;
    const int32_t lim = use138 ? lim138 : lim134;

    int32_t d = static_cast<int32_t>(static_cast<uint32_t>(lim) - static_cast<uint32_t>(centre));
    int32_t v = pos;
    if (d < 0) {
        if (v < lim) v = lim;
        if (v > centre) v = centre;
    } else {
        if (v < centre) v = centre;
        if (v > lim) v = lim;
    }
    if (d == 0) d = 1;
    const int32_t num =
        static_cast<int32_t>((static_cast<uint32_t>(v) - static_cast<uint32_t>(centre)) << 14);
    const int32_t q = num / d;
    const int32_t rem = num % d;

    const int32_t c = clamp_3f80(q);
    int32_t res;
    if (use138) {
        res = static_cast<int32_t>(0u - static_cast<uint32_t>(c)) >> 2;
    } else {
        res = c >> 2;
    }
    r->eax = static_cast<uint32_t>(res);
    r->ecx = static_cast<uint32_t>(d);
    r->edx = static_cast<uint32_t>(rem);

    const int16_t ax = static_cast<int16_t>(res);
    GP4_FIELD(int16_t, rec, 0x6a) = ax;
    if (ax >= 0x800) {
        GP4_FIELD(uint8_t, rec, 0x6f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x6f) | 8u);
    }
    uint32_t fl;
    if (ax < -0x800) {
        const uint8_t b = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x6f) | 4u);
        GP4_FIELD(uint8_t, rec, 0x6f) = b;
        fl = gp4::flags::logic8(b);
    } else {
        fl = gp4::flags::sub16(static_cast<uint16_t>(ax), 0xf800u);
    }
    r->eflags = gp4::flags::merge(r->eflags, fl);
}
GP4_IMPL(0x0051de03, FUN_0051de03, "regs(esi:ptr[0x78]:bytes, eax:u32, ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctl_007ab548+axis_0051de03")
