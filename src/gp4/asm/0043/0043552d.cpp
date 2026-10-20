#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte logic result (test / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Arithmetic flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// Hands the registers and flags a called routine leaves back to the caller's register file.
static inline void takeResult(gp4::Regs* r, const gp4::Regs& call) {
    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}

// 0x00435691: runs the car reset 0x0040f6d0, then reloads edi from the car's segment (+0x1c).
static void restartCar(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0040f6d0, call);
    takeResult(r, call);
    r->edi = GP4_FIELD(uint32_t, car, 0x1c);
}

// 0x0043563b: the car is not on the pit-lane approach. Restart it only when it is sliding
// (+0x1a0 < 0) and its lateral offset |+0x90| exceeds the allowed band +0xb0 + 0xb0 (+0x100 more
// with bit 4 of +0x179), and then only if the pit-lane test 0x4345ab of the segment says "outside".
static void checkLateralOffset(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t slip = GP4_FIELD(int32_t, car, 0x1a0);
    if (slip >= 0) {
        setFlags(r, cmpFlags32(static_cast<uint32_t>(slip), 0u));
        return;
    }
    uint32_t offset = GP4_FIELD(uint16_t, car, 0x90);
    if ((offset & 0x8000u) != 0u) {
        offset = (0u - offset) & 0xffffu;
    }
    uint32_t band = (GP4_FIELD(uint16_t, car, 0xb0) + 0xb0u) & 0xffffu;
    r->eax = (r->eax & 0xffff0000u) | offset;
    r->edx = (r->edx & 0xffff0000u) | band;
    setFlags(r, cmpFlags16(offset, band));
    if (static_cast<int16_t>(offset) <= static_cast<int16_t>(band)) {
        return;
    }
    if ((GP4_FIELD(uint8_t, car, 0x179) & 0x10u) != 0u) {
        band = (band + 0x100u) & 0xffffu;
        r->edx = (r->edx & 0xffff0000u) | band;
        setFlags(r, cmpFlags16(offset, band));
        if (static_cast<int16_t>(offset) <= static_cast<int16_t>(band)) {
            return;
        }
    }
    if ((GP4_FIELD(uint8_t, car, 0x9c) & 0x10u) != 0u && (GP4_FIELD(uint8_t, segment, 0x1b) & 0x20u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x004345ab, call);
        takeResult(r, call);
        if ((r->eflags & 1u) == 0u) {
            return;
        }
    }
    restartCar(r);
}

// FUNCTION: GP4 0x0043552d
// Decides whether the car in esi (edi = the track segment it is on) must be reset by 0x0040f6d0.
// With bit 7 of +0x26b set the cosine band of the heading difference (word +0xe against the
// segment's first word) is compared with the lateral offset: restart when the offset falls short of
// +0x11e (or +0x11c, by the sign of byte [0x868582]) + 0x150. Otherwise a car whose +0x161 bit 1
// gate passes 0x434d6b restarts at once; with bit 0 of +0x179 set the car is released when bit 7 of
// +0x71 is set, or armed (+0x1f0 = clock + 3000) and restarted while the segment's byte +0x71 lies
// in [0x602b44, 0x602b48]; everything else goes through the lateral-offset test above.
// eax / edx / edi and the flags are those of the last operation on the taken path.
void __cdecl FUN_0043552d(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    if ((GP4_FIELD(uint8_t, car, 0x26b) & 0x80u) != 0u) {
        // |heading difference| folded through the cosine table (Q14 words at 0x7c5c5c)
        const uint16_t difference = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0xe) - GP4_FIELD(uint16_t, segment, 0));
        uint32_t ax = (static_cast<int16_t>(difference) >= 0) ? ((0u - difference) & 0xffffu) : difference;
        ax = (ax + 0x4000u) & 0xffffu;
        if ((ax & 0x8000u) != 0u) {
            ax = (0u - ax) & 0xffffu;
        }
        const uint32_t index = (ax >> 2) & 0xfffeu;
        const int32_t cosine = GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), index);
        const uint32_t scaled = static_cast<uint32_t>(cosine >> 6);
        uint32_t lo = (scaled + 0xbdu) & 0xffffu;
        uint32_t limit;
        if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
            limit = (GP4_FIELD(uint16_t, car, 0x11e) + 0x150u) & 0xffffu;
            lo = (lo + GP4_FIELD(uint16_t, car, 0x90)) & 0xffffu;
        } else {
            limit = (GP4_FIELD(uint16_t, car, 0x11c) + 0x150u) & 0xffffu;
            lo = (lo - GP4_FIELD(uint16_t, car, 0x90)) & 0xffffu;
        }
        r->eax = (scaled & 0xffff0000u) | lo;
        r->edx = (r->edx & 0xffff0000u) | limit;
        setFlags(r, cmpFlags16(lo, limit));
        if (static_cast<int16_t>(lo) >= static_cast<int16_t>(limit)) {
            restartCar(r);
        }
        return;
    }

    if ((GP4_FIELD(uint8_t, car, 0x161) & 2u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00434d6b, call);
        takeResult(r, call);
        if ((r->eflags & 1u) != 0u) {
            restartCar(r);
            return;
        }
    }

    if ((GP4_FIELD(uint8_t, car, 0x179) & 1u) != 0u) {
        if ((GP4_FIELD(uint8_t, car, 0x71) & 0x80u) != 0u) {
            GP4_FIELD(uint8_t, car, 0x179) &= 0xfe;
            GP4_FIELD(uint8_t, car, 0x203) &= 0xdf;
            setFlags(r, logicFlags8(GP4_FIELD(uint8_t, car, 0x203)));
            return;
        }
        const int32_t zone = GP4_FIELD(uint8_t, segment, 0x71);
        r->edx = static_cast<uint32_t>(zone);
        const int32_t zoneLow = GP4_GLOBAL(int32_t, 0x00602b44);
        const int32_t zoneHigh = GP4_GLOBAL(int32_t, 0x00602b48);
        if (zone >= zoneLow && zone <= zoneHigh) {
            int32_t gear = GP4_FIELD(uint8_t, car, 0x41b);
            if (gear > zoneHigh) {
                gear = zoneHigh;
            }
            gear -= 1;
            r->eax = static_cast<uint32_t>(gear);
            if (gear <= zone) {
                GP4_FIELD(uint32_t, car, 0x1f0) = GP4_GLOBAL(uint32_t, 0x0066d814);
                GP4_FIELD(uint32_t, car, 0x1f0) += 0xbb8u;
                restartCar(r);
                return;
            }
        }
    }
    checkLateralOffset(r);
}
GP4_IMPL(0x0043552d, FUN_0043552d, "regs(eax:u32, edx:u32, esi:ptr[0x420]:layout=car_004129fa+rec_00412b88, edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_0040705f+frac_0040705f+state_00407af6+state_004381ab+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+state_0040759e+trackfar_008943a0+flag_00604418")
