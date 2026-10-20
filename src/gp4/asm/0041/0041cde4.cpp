#include <gp4/gp4.h>
#include <gp4/flags.h>

// imul followed by `shrd eax, edx, 14`: bits 14..45 of the signed 64-bit product.
static inline uint32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Low dword of a signed imul.
static inline uint32_t mulLow(int32_t a, int32_t b) {
    return static_cast<uint32_t>(a) * static_cast<uint32_t>(b);
}

// Clamp to [ [0x00602010], [0x00602014] ]: the upper bound is tested first (jle), then the lower (jge).
static inline int32_t clampSteer(int32_t value) {
    if (value > GP4_GLOBAL(int32_t, 0x00602014)) {
        value = GP4_GLOBAL(int32_t, 0x00602014);
    }
    if (value < GP4_GLOBAL(int32_t, 0x00602010)) {
        value = GP4_GLOBAL(int32_t, 0x00602010);
    }
    return value;
}

// (imul([0x00638010], 0x28be) >> 8) sign-extended, idiv by the divisor (raised to at least [0x0060200c]),
// which is also published at 0x006224bd; the quotient goes to 0x00638de0.
static inline void scaleSpeed(int32_t divisor) {
    const int32_t speed = GP4_GLOBAL(int32_t, 0x00638010);
    if (GP4_GLOBAL(uint8_t, 0x006246e1) != 0u) {
        GP4_GLOBAL(int32_t, 0x006224b9) = speed;
    }
    const int64_t product = static_cast<int64_t>(speed) * 0x28be;
    const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 8));
    if (divisor < GP4_GLOBAL(int32_t, 0x0060200c)) {
        divisor = GP4_GLOBAL(int32_t, 0x0060200c);
    }
    GP4_GLOBAL(int32_t, 0x006224bd) = divisor;
    int32_t remainder;
    GP4_GLOBAL(int32_t, 0x00638de0) = gp4::idiv64(static_cast<int64_t>(scaled), divisor, &remainder);
}

// pushal ... popal: track-relative frame of the car in esi. Byte 0x00638d60 = 0xff when bit 1 of +0x8d or
// bit 0 of +0x1ce is set (else 0); the car position (+0, +4) goes to 0x00623e30/0x00623e38 and 0x0041e069
// locates it on the track with index 4; 0x00637fe4 = [0x00623e34] + ([0x00602004] >> 4). With bit 0 of
// +0x1ce the track state is the source (segment [0x00602008], lateral [0x006278b0], parameter
// [0x006278b4], speed divisor [0x00602024] >> 8, steer [0x00602020]); otherwise the car's own segment
// +0x1c, lateral word +0x90 (<< 8), parameter word +0x66 (clamped) and divisor word +0x146. 0x0041ec08
// and 0x0041e9ab evaluate the segment; [0x00638010] * 0x28be >> 8 over the divisor (at least
// [0x0060200c]) gives 0x00638de0. The steer is clamped to [0x00602010..0x00602014]; it lerps the segment
// words +0x26 -> next +0x26 (0x00602070, faded by 0x0041e019's offset beyond [0x00602068] into
// 0x00638de4) and +0 -> next +0 (0x00638ddc). The heading (+0xc >> 16) minus that angle is turned into
// sine/cosine by 0x0040119c; the two rotated components (<< 2) go to 0x00638dd4/0x00638dd8 and their
// high words to +0x1c0/+0x1c2 when byte 0x006246e1 is set. 0x00401fb4 (byte 0x00625990 == 0xff),
// 0x0040161d and 0x004016ba rebuild the orientation; word +0x1be = ([0x00637fe4] - +8) >> 6. Only the
// flags of that final `sar` escape.
// FUNCTION: GP4 0x0041cde4
void __cdecl FUN_0041cde4(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint8_t latch = 0xffu;
    if ((car[0x8d] & 0x02u) == 0u && (car[0x1ce] & 0x01u) == 0u) {
        latch = 0u;
    }
    c.eax = (c.eax & 0xffffff00u) | latch;
    GP4_GLOBAL(uint8_t, 0x00638d60) = latch;
    GP4_GLOBAL(uint32_t, 0x00623e30) = GP4_FIELD(uint32_t, car, 0x0);
    c.eax = GP4_FIELD(uint32_t, car, 0x4);
    GP4_GLOBAL(uint32_t, 0x00623e38) = c.eax;
    GP4_GLOBAL(uint32_t, 0x0062789c) = 4u;
    gp4::call_regs(0x0041e069, c);

    c.edx = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00602004) >> 4);
    c.eax = GP4_GLOBAL(uint32_t, 0x00623e34) + c.edx;
    GP4_GLOBAL(uint32_t, 0x00637fe4) = c.eax;

    int32_t steer;
    if ((car[0x1ce] & 0x01u) != 0u) {
        c.edi = GP4_GLOBAL(uint32_t, 0x00602008);
        c.eax = GP4_GLOBAL(uint32_t, 0x006278b4);
        c.edx = GP4_GLOBAL(uint32_t, 0x006278b0);
        gp4::call_regs(0x0041ec08, c);
        gp4::call_regs(0x0041e9ab, c);
        scaleSpeed(GP4_GLOBAL(int32_t, 0x00602024) >> 8);
        GP4_GLOBAL(uint32_t, 0x00602028) = GP4_GLOBAL(uint32_t, 0x006278b0);
        steer = GP4_GLOBAL(int32_t, 0x00602020);
    } else {
        c.edi = GP4_FIELD(uint32_t, car, 0x1c);
        c.eax = static_cast<uint32_t>(clampSteer(GP4_FIELD(int16_t, car, 0x66)));
        c.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x90))) << 8;
        gp4::call_regs(0x0041ec08, c);
        gp4::call_regs(0x0041e9ab, c);
        scaleSpeed(GP4_FIELD(int16_t, car, 0x146));
        GP4_GLOBAL(uint32_t, 0x00602028) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x90))) << 8;
        steer = GP4_FIELD(int16_t, car, 0x66);
    }
    steer = clampSteer(steer);
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi));

    c.eax = GP4_GLOBAL(uint32_t, 0x00602028);
    gp4::call_regs(0x0041e019, c);
    int32_t beyond = static_cast<int32_t>(c.eax - GP4_GLOBAL(uint32_t, 0x00602068));
    if (beyond < 0) beyond = 0;
    if (beyond > 0x20000) beyond = 0x20000;
    const int32_t fade = static_cast<int32_t>(0x4000u - (static_cast<uint32_t>(beyond) >> 3));

    const int16_t bankNow = GP4_FIELD(int16_t, segment, 0x26);
    const int16_t bankNext = GP4_FIELD(int16_t, segment, 0x186);
    const uint32_t bank = mulQ14(static_cast<int16_t>(bankNext - bankNow), steer) + static_cast<uint32_t>(static_cast<int32_t>(bankNow));
    GP4_GLOBAL(uint32_t, 0x00602070) = bank;
    GP4_GLOBAL(uint32_t, 0x00638de4) = mulQ14(static_cast<int32_t>(bank), fade);

    const int16_t angleNow = GP4_FIELD(int16_t, segment, 0x0);
    const int16_t angleNext = GP4_FIELD(int16_t, segment, 0x160);
    const uint32_t angle = mulQ14(static_cast<int16_t>(angleNext - angleNow), steer) + static_cast<uint32_t>(static_cast<int32_t>(angleNow));
    GP4_GLOBAL(uint32_t, 0x00638ddc) = angle;

    const uint32_t heading = GP4_FIELD(uint32_t, car, 0xc);
    GP4_GLOBAL(uint32_t, 0x00638dd0) = heading;
    const uint16_t relative = static_cast<uint16_t>((heading >> 16) - angle);

    c.eax = (angle & 0xffff0000u) | relative;
    gp4::call_regs(0x0040119c, c);
    const int32_t sine = static_cast<int16_t>(c.eax);
    c.eax = (angle & 0xffff0000u) | static_cast<uint16_t>(0x4000u - relative);
    gp4::call_regs(0x0040119c, c);
    const int32_t cosine = static_cast<int16_t>(c.eax);

    const uint32_t across = (mulLow(GP4_GLOBAL(int32_t, 0x00638de0), sine)
        - mulLow(GP4_GLOBAL(int32_t, 0x00638de4), cosine)) << 2;
    if (GP4_GLOBAL(uint8_t, 0x006246e1) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00638dd4) = across;
        GP4_FIELD(uint16_t, car, 0x1c0) = static_cast<uint16_t>(across >> 16);
    }

    c.eax = GP4_GLOBAL(uint32_t, 0x00602028);
    gp4::call_regs(0x0041e019, c);
    int32_t past = static_cast<int32_t>(c.eax - GP4_GLOBAL(uint32_t, 0x0060206c));
    if (past < 0) past = 0;
    if (past > 0x10000) past = 0x10000;
    const int32_t fade2 = static_cast<int32_t>(0x4000u - (static_cast<uint32_t>(past) >> 2));
    const uint32_t lean = mulLow(static_cast<int32_t>(mulQ14(fade2, GP4_GLOBAL(int32_t, 0x00602070))), sine);
    const uint32_t along = (mulLow(GP4_GLOBAL(int32_t, 0x00638de0), cosine) + lean) << 2;
    if (GP4_GLOBAL(uint8_t, 0x006246e1) != 0u) {
        GP4_GLOBAL(uint32_t, 0x00638dd8) = along;
        GP4_FIELD(uint16_t, car, 0x1c2) = static_cast<uint16_t>(along >> 16);
    }

    if (GP4_GLOBAL(uint8_t, 0x00625990) == 0xffu) {
        gp4::call_regs(0x00401fb4, c);
    }
    gp4::call_regs(0x0040161d, c);
    gp4::call_regs(0x004016ba, c);

    const uint32_t lift = GP4_GLOBAL(uint32_t, 0x00637fe4) - GP4_FIELD(uint32_t, car, 0x8);
    const int32_t height = static_cast<int32_t>(lift) >> 6;
    GP4_FIELD(uint16_t, car, 0x1be) = static_cast<uint16_t>(height);

    // sar by 6: CF = the last bit shifted out, OF = AF = 0.
    const uint32_t flags = ((lift >> 5) & 1u) | gp4::flags::logic32(static_cast<uint32_t>(height));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0041cde4, FUN_0041cde4, "regs(esi:ptr[0x238]:layout=car_0043c677+rec_004295fe) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+multiply_0040ce7b+words_008943a0+state_004094e3+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_004381ab+trackgeo_008943a0+state_0040705f+frac_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_0041dbb0+state_0040342a+tbl_00868598+steps_0041dada+state_0041d996+state_0041d9b6+mesh_00453330+state_0041d9eb+state_0041da34+ptr_00602008+ptr_00602008+win_0041e069; maxinsn=16000000; wall=1800")
