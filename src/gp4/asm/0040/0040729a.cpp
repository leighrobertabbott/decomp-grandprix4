#include <gp4/gp4.h>
#include <gp4/x87.h>

// (sx word << 8) + dword: a record point in the 8-bit sub-unit frame.
static inline uint32_t point(const uint8_t* rec, uint32_t wordOff, uint32_t dwordOff) {
    return (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + wordOff))) << 8)
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

// Segment record in edi (and the next record at +0x160). Builds the two end-point pairs of the segment
// (x: 0x6279ac/b0 .. b8/b4, z: 0x6279cc/d0 .. d8/d4, plus the +0x108/+0x268 offset points 0x6279c0/c4),
// fits the car position 0x6278c0/0x6278c8 with 0x00407af6 (-> 0x6278bc and the fractions 0x627950/58),
// returns eax = lerp of the offset points + 0x00406f05 + 0x0040705f (edx = -1 for both), stores the Q14
// fractions 0x6278b4/0x6278b8 and the lateral offset 0x6278b0 (the car position rotated by word +0x128,
// minus (u16 +0x60 + 0x50) << 8), and runs 0x0041ec08 on them; the flags are those 0x0041ec08 leaves.
// FUNCTION: GP4 0x0040729a
void __cdecl FUN_0040729a(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(uint32_t, 0x006279ac) = point(rec, 0xd4, 0xf0);
    GP4_GLOBAL(uint32_t, 0x006279bc) = point(rec, 0xd8, 0xf4);
    GP4_GLOBAL(uint32_t, 0x006279cc) = point(rec, 0xdc, 0xf8);
    GP4_GLOBAL(uint32_t, 0x006279b0) = *reinterpret_cast<const uint32_t*>(rec + 0x118);
    GP4_GLOBAL(uint32_t, 0x006279c0) =
        (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + 0xd8))) << 8)
        + (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + 0x108))) << 6)
        + *reinterpret_cast<const uint32_t*>(rec + 0xf4);
    GP4_GLOBAL(uint32_t, 0x006279d0) = *reinterpret_cast<const uint32_t*>(rec + 0x11c);
    GP4_GLOBAL(uint32_t, 0x006279b4) = *reinterpret_cast<const uint32_t*>(rec + 0x278);
    GP4_GLOBAL(uint32_t, 0x006279c4) =
        (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + 0x238))) << 8)
        + (static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(rec + 0x268))) << 6)
        + *reinterpret_cast<const uint32_t*>(rec + 0x254);
    GP4_GLOBAL(uint32_t, 0x006279d4) = *reinterpret_cast<const uint32_t*>(rec + 0x27c);
    GP4_GLOBAL(uint32_t, 0x006279b8) = point(rec, 0x234, 0x250);
    GP4_GLOBAL(uint32_t, 0x006279c8) = point(rec, 0x238, 0x254);
    GP4_GLOBAL(uint32_t, 0x006279d8) = point(rec, 0x23c, 0x258);

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
    c.edx = 0xffffffffu;
    gp4::call_regs(0x00406f05, c);
    sum += c.eax;

    c.st_in = 0;
    c.st_out = 0;
    c.eax = sum;
    c.edx = 0xffffffffu;
    gp4::call_regs(0x0040705f, c);
    sum += c.eax;

    GP4_GLOBAL(int32_t, 0x006278b4) =
        fistp32(ext(GP4_GLOBAL(double, 0x00627950)) * ext(GP4_GLOBAL(double, 0x006f2b04)));
    GP4_GLOBAL(int32_t, 0x006278b8) =
        fistp32(ext(GP4_GLOBAL(double, 0x00627958)) * ext(GP4_GLOBAL(double, 0x006f2b04)));

    const int32_t angle = *reinterpret_cast<const int16_t*>(rec + 0x128);
    Ext sine, cosine;
    fsincos(ext_i32(angle) * ext(GP4_GLOBAL(double, 0x006f2bc4)), &sine, &cosine);
    const uint32_t dx = GP4_GLOBAL(uint32_t, 0x006278c0) - GP4_GLOBAL(uint32_t, 0x006279ac);
    const Ext along = ext_i32(static_cast<int32_t>(dx)) * cosine;
    const uint32_t dz = GP4_GLOBAL(uint32_t, 0x006278c8) - GP4_GLOBAL(uint32_t, 0x006279cc);
    const Ext across = fchs(ext_i32(static_cast<int32_t>(dz)) * sine);
    const int32_t lateral = fistp32(along + across);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = lateral;
    const uint32_t edge = (static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(rec + 0x60)) + 0x50u) << 8;
    GP4_GLOBAL(uint32_t, 0x006278b0) = static_cast<uint32_t>(lateral) - edge;

    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint32_t, 0x006278b4);
    c.edx = GP4_GLOBAL(uint32_t, 0x006278b0);
    gp4::call_regs(0x0041ec08, c);

    takeRegs(r, c);
    r->eax = sum;
}
GP4_IMPL(0x0040729a, FUN_0040729a, "regs(edi:rec[0x8943a0,0x160,0..11]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_004381ab+ring_00406ac7+trackgeo_008943a0+state_0040705f+frac_0040705f+state_00407af6+state_0040759e+trackfar_008943a0")
