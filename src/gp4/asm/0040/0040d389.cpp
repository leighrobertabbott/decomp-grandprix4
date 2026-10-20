#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of `sar value, 6` in the oracle model: CF = last bit out, OF = AF = 0.
static inline uint32_t sar6Flags(uint32_t value) {
    const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(value) >> 6);
    return ((value >> 5) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Segment-ring "link" record (esi/edi of the caller loop, 0x160 bytes apiece).
struct Unknown_0040d389_Link {
    uint8_t Unknown00[0x44];
    uint32_t Cursor44;
    uint8_t Unknown48[0x7c - 0x48];
    int8_t First7C;
    int8_t Last7D;
    uint8_t Unknown7E[0x104 - 0x7e];
    int32_t Angle104;
    uint8_t Unknown108[0x110 - 0x108];
    int32_t Angle110;
    uint8_t Unknown114[0x154 - 0x114];
    uint16_t Result154;
    int16_t Result156;
    uint16_t Result158;
    int16_t Result15A;
};

// FUNCTION: GP4 0x0040d389
// pushal/popal wrapper: for a link whose cursor +0x44 is set, whose cursor distance to [0x62d2d4]
// is negative or exceeds the one from [0x62d2d8], and whose signed bytes +0x7c <= +0x7d, it
// builds the axis (cos, sin) of the angle +0x104 + 0x40000000 (or +0x110 - 0x40000000 when byte
// [0x868582] is non-negative) scaled by [0x6f2b04] into [0x62780c]/[0x627810], takes the point of
// 0x0040d17c as origin ([0x6277f8], [0x6277fc], [0x627800]) and runs 0x0040d19f over the records
// cursor + (+0x7c)*0x160 - 0x2c0 .. cursor + (+0x7d)*0x160 + 0x420 for both sides (byte [0x627808]
// 0xff then 0), storing the (offset, scaled distance) word pairs at +0x154/+0x156 and +0x158/+0x15a.
// All registers are restored; the flags are those of the last flag writer before popal.
void __cdecl FUN_0040d389(gp4::Regs* r) {
    Unknown_0040d389_Link* link = reinterpret_cast<Unknown_0040d389_Link*>(static_cast<uintptr_t>(r->edi));
    uint32_t flags;

    const uint32_t cursor = link->Cursor44;
    if (cursor == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;  // or ebx, ebx
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x0062d2d8);
    c.edx = GP4_GLOBAL(uint32_t, 0x0062d2d4);
    gp4::call_regs(0x004062b1, c);
    const int32_t reference = static_cast<int32_t>(c.eax);

    c.eax = r->edi;
    c.edx = GP4_GLOBAL(uint32_t, 0x0062d2d4);
    gp4::call_regs(0x004062b1, c);
    const int32_t distance = static_cast<int32_t>(c.eax);

    flags = cmpFlags32(static_cast<uint32_t>(distance), 0u);
    if (distance >= 0) {
        flags = cmpFlags32(static_cast<uint32_t>(distance), static_cast<uint32_t>(reference));
        if (distance <= reference) {
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
    }

    const int32_t last = link->Last7D;
    const int32_t first = link->First7C;
    flags = cmpFlags32(static_cast<uint32_t>(last), static_cast<uint32_t>(first));
    if (last < first) {
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    uint32_t angle;
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) {
        angle = static_cast<uint32_t>(link->Angle104) + 0x40000000u;
    } else {
        angle = static_cast<uint32_t>(link->Angle110) - 0x40000000u;
    }
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = angle;

    const Ext theta = gp4::x87::ext_i32(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006f2bf4)))
        * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    gp4::x87::fsincos(theta, &sine, &cosine);
    GP4_GLOBAL(int32_t, 0x0062780c) = gp4::x87::fistp32(cosine * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_GLOBAL(int32_t, 0x00627810) = gp4::x87::fistp32(sine * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b04)));

    c.eax = angle & ~0xffu;  // mov al, 0
    c.edi = r->edi;
    gp4::call_regs(0x0040d17c, c);
    GP4_GLOBAL(uint32_t, 0x006277f8) = c.eax;
    GP4_GLOBAL(uint32_t, 0x006277fc) = c.ecx;
    GP4_GLOBAL(uint32_t, 0x00627800) = c.edx;
    GP4_GLOBAL(uint32_t, 0x00627804) = r->edi;

    const uint32_t walkEnd = static_cast<uint32_t>(last) * 0x160u + cursor + 0x2c0u + 0x160u;
    const uint32_t walkStart = static_cast<uint32_t>(first) * 0x160u - 0x2c0u + cursor;
    c.ebx = walkEnd;
    c.edi = walkStart;
    c.eax = walkStart;
    // edx keeps the y value just returned by 0x0040d17c.

    GP4_GLOBAL(uint8_t, 0x00627808) = 0xffu;
    gp4::call_regs(0x0040d19f, c);
    if ((c.eflags & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
        return;
    }

    uint8_t* target = GP4_GLOBAL(uint8_t*, 0x00627804);
    GP4_FIELD(uint16_t, target, 0x154) = static_cast<uint16_t>(c.eax);
    c.edx = static_cast<uint32_t>(static_cast<int32_t>(c.edx) >> 6);
    GP4_FIELD(uint16_t, target, 0x156) = static_cast<uint16_t>(c.edx);

    GP4_GLOBAL(uint8_t, 0x00627808) = 0u;
    gp4::call_regs(0x0040d19f, c);
    if ((c.eflags & 1u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
        return;
    }

    target = GP4_GLOBAL(uint8_t*, 0x00627804);
    GP4_FIELD(uint16_t, target, 0x158) = static_cast<uint16_t>(c.eax);
    GP4_FIELD(uint16_t, target, 0x15a) = static_cast<uint16_t>(static_cast<int32_t>(c.edx) >> 6);
    r->eflags = (r->eflags & ~0x8d5u) | sar6Flags(c.edx);
}
GP4_IMPL(0x0040d389, FUN_0040d389, "regs(edi:rec[0x8943a0,0x160,0..7]) -> (cf, pf, af, zf, sf, of); globals=layout=state_0040d19f+state_0040d389")
