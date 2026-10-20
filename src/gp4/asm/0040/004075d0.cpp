#include <gp4/gp4.h>
#include <gp4/x87.h>

// (sx word << 8) + dword: a record point in the 8-bit sub-unit frame.
static inline uint32_t point(const uint8_t* rec, uint32_t wordOff, uint32_t dwordOff) {
    return (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + wordOff))) << 8)
        + *reinterpret_cast<const uint32_t*>(rec + dwordOff);
}

// (sx word << 8) + (sx offset word << 6) + dword: an offset point.
static inline uint32_t offsetPoint(const uint8_t* rec, uint32_t wordOff, uint32_t offsetOff, uint32_t dwordOff) {
    return (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + wordOff))) << 8)
        + (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + offsetOff))) << 6)
        + *reinterpret_cast<const uint32_t*>(rec + dwordOff);
}

// fild dword [0x6f2bf4]; fmul qword [fraction]; fistp dword [0x6f2bf4].
static inline int32_t scaleByFraction(uint32_t value, double fraction) {
    using namespace gp4::x87;
    const int32_t scaled = fistp32(ext_i32(static_cast<int32_t>(value)) * ext(fraction));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;
    return scaled;
}

static inline void takeRegs(gp4::Regs* r, const gp4::Regs& c) {
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}

// Mirror of 0x0040729a for the other direction: the segment end points are taken from the next record
// (edi + 0x160) back to this one (+0xd6/+0xda/+0xde words, +0x114 offset, +0x120/+0x124 dwords), the
// callees get edx = 0, the Q14 fraction 0x6278b4 is 1 - [0x627950], the rotation uses word +0x12a and the
// lateral offset adds (u16 +0x62 + 0x50) << 8; the flags are those 0x0041ec08 leaves.
// FUNCTION: GP4 0x004075d0
void __cdecl FUN_004075d0(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(uint32_t, 0x006279ac) = point(rec, 0x236, 0x250);
    GP4_GLOBAL(uint32_t, 0x006279bc) = point(rec, 0x23a, 0x254);
    GP4_GLOBAL(uint32_t, 0x006279cc) = point(rec, 0x23e, 0x258);
    GP4_GLOBAL(uint32_t, 0x006279b0) = *reinterpret_cast<const uint32_t*>(rec + 0x280);
    GP4_GLOBAL(uint32_t, 0x006279c0) = offsetPoint(rec, 0x23a, 0x274, 0x254);
    GP4_GLOBAL(uint32_t, 0x006279d0) = *reinterpret_cast<const uint32_t*>(rec + 0x284);
    GP4_GLOBAL(uint32_t, 0x006279b4) = *reinterpret_cast<const uint32_t*>(rec + 0x120);
    GP4_GLOBAL(uint32_t, 0x006279c4) = offsetPoint(rec, 0xda, 0x114, 0xf4);
    GP4_GLOBAL(uint32_t, 0x006279d4) = *reinterpret_cast<const uint32_t*>(rec + 0x124);
    GP4_GLOBAL(uint32_t, 0x006279b8) = point(rec, 0xd6, 0xf0);
    GP4_GLOBAL(uint32_t, 0x006279c8) = point(rec, 0xda, 0xf4);
    GP4_GLOBAL(uint32_t, 0x006279d8) = point(rec, 0xde, 0xf8);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x006278c0);
    c.edx = GP4_GLOBAL(uint32_t, 0x006278c8);
    gp4::call_regs(0x00407af6, c);
    GP4_GLOBAL(uint32_t, 0x006278bc) = c.eax;

    // Lerp of the two offset points along the segment.
    const uint32_t near0 = GP4_GLOBAL(uint32_t, 0x006279bc);
    const uint32_t start = static_cast<uint32_t>(scaleByFraction(GP4_GLOBAL(uint32_t, 0x006279c0) - near0,
                                                                 GP4_GLOBAL(double, 0x00627958))) + near0;
    const uint32_t near1 = GP4_GLOBAL(uint32_t, 0x006279c8);
    const uint32_t end = static_cast<uint32_t>(scaleByFraction(GP4_GLOBAL(uint32_t, 0x006279c4) - near1,
                                                               GP4_GLOBAL(double, 0x00627958))) + near1;
    uint32_t sum = start + static_cast<uint32_t>(scaleByFraction(end - start, GP4_GLOBAL(double, 0x00627950)));

    c.st_in = 0;
    c.st_out = 0;
    c.eax = sum;
    c.edx = 0u;
    gp4::call_regs(0x00406f05, c);
    sum += c.eax;

    c.st_in = 0;
    c.st_out = 0;
    c.eax = sum;
    c.edx = 0u;
    gp4::call_regs(0x0040705f, c);
    sum += c.eax;

    GP4_GLOBAL(int32_t, 0x006278b4) =
        fistp32((ext_one() - ext(GP4_GLOBAL(double, 0x00627950))) * ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_GLOBAL(int32_t, 0x006278b8) =
        fistp32(ext(GP4_GLOBAL(double, 0x00627958)) * ext(GP4_GLOBAL(double, 0x006f2b04)));

    const int32_t angle = *reinterpret_cast<const int16_t*>(rec + 0x12a);
    Ext sine, cosine;
    fsincos(ext_i32(angle) * ext(GP4_GLOBAL(double, 0x006f2bc4)), &sine, &cosine);
    const uint32_t dx = GP4_GLOBAL(uint32_t, 0x006278c0) - GP4_GLOBAL(uint32_t, 0x006279b8);
    const Ext along = ext_i32(static_cast<int32_t>(dx)) * cosine;
    const uint32_t dz = GP4_GLOBAL(uint32_t, 0x006278c8) - GP4_GLOBAL(uint32_t, 0x006279d8);
    const Ext across = fchs(ext_i32(static_cast<int32_t>(dz)) * sine);
    const int32_t lateral = fistp32(along + across);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = lateral;
    const uint32_t edge = (static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(rec + 0x62)) + 0x50u) << 8;
    GP4_GLOBAL(uint32_t, 0x006278b0) = static_cast<uint32_t>(lateral) + edge;

    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x006278b4);
    c.edx = GP4_GLOBAL(uint32_t, 0x006278b0);
    gp4::call_regs(0x0041ec08, c);

    takeRegs(r, c);
    r->eax = sum;
}
GP4_IMPL(0x004075d0, FUN_004075d0, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_004381ab+ring_00406ac7+trackgeo_008943a0+state_0040705f+frac_0040705f+state_00407af6+state_0040759e+trackfar_008943a0")
