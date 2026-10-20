#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `cmp left, right`; operands already masked to 16 bits.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041416e
// Steering update of the car at esi (heading word +0xe, edi = its track segment). With a target
// (byte +0x235 >= 2, 0x00414439 gives the bearing eax and distance edx) closer than dword 0x6224fc
// and within 0x4000 of the heading, bit 0 of +0x6f is set (ebp = bearing, +0x72 = 0xff); otherwise,
// when |word +0x9a| < 0xbc, +0x72 = 1 and the routine returns CF = 1, else bit 1 of +0x6f is set and
// +0x72 = 0xff. The steering word +0xb4 is then the Q14 cosine table (0x7c5c5c) of the heading
// difference to ebp with the sign of the difference (or, when word 0x934df8 < 0 and word 0x934df4 <
// 0x1800 and |word +0x90| < word +0xb0, the angle 0x0041447e minus the heading minus 0x8000),
// clamped to +-dword 0x6013ac; byte 0x603c34 = 0xff and bit 4 / 8 of +0x6f is set for |steering| >= 0x80
// by its sign; CF = 0. eax = steering, edx = |steering| (or the word-wise edx values of the early
// exit), ebp = the bearing; ebx, ecx, esi, edi are preserved.
void __cdecl FUN_0041416e(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    bool fallback = false;

    if (GP4_FIELD(uint8_t, car, 0x235) < 2u) {
        fallback = true;
    } else {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x00414439, call);
        eax = call.eax;
        edx = call.edx;
        if ((call.eflags & 1u) != 0u) {
            fallback = true;
        } else if (static_cast<int32_t>(edx) >= GP4_GLOBAL(int32_t, 0x006224fc)) {
            fallback = true;
        } else {
            ebp = eax;
            uint16_t gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x0e) - static_cast<uint16_t>(eax));
            if (static_cast<int16_t>(gap) < 0) {
                gap = static_cast<uint16_t>(0u - gap);
            }
            edx = (edx & 0xffff0000u) | gap;
            if (static_cast<int16_t>(gap) >= 0x4000) {
                fallback = true;
            } else {
                GP4_FIELD(uint8_t, car, 0x6f) |= 1u;
                GP4_FIELD(uint8_t, car, 0x72) = 0xffu;
            }
        }
    }

    if (fallback) {
        uint16_t slip = GP4_FIELD(uint16_t, car, 0x9a);
        if (static_cast<int16_t>(slip) < 0) {
            slip = static_cast<uint16_t>(0u - slip);
        }
        edx = (edx & 0xffff0000u) | slip;
        if (static_cast<int16_t>(slip) < 0xbc) {
            GP4_FIELD(uint8_t, car, 0x72) = 1u;
            r->eax = eax;
            r->edx = edx;
            r->ebp = ebp;
            r->eflags = (r->eflags & ~0x8d5u) | (subFlags16(slip, 0xbcu) & ~1u) | 1u;
            return;
        }
        GP4_FIELD(uint8_t, car, 0x6f) |= 2u;
        GP4_FIELD(uint8_t, car, 0x72) = 0xffu;
    }

    bool viaAngle = false;
    if (GP4_GLOBAL(int16_t, 0x00934df8) < 0 && GP4_GLOBAL(int16_t, 0x00934df4) < 0x1800) {
        uint16_t sway = GP4_FIELD(uint16_t, car, 0x90);
        if (static_cast<int16_t>(sway) < 0) {
            sway = static_cast<uint16_t>(0u - sway);
        }
        viaAngle = static_cast<int16_t>(sway) < GP4_FIELD(int16_t, car, 0xb0);
    }

    if (viaAngle) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = eax;
        call.edx = edx;
        call.ebp = ebp;
        gp4::call_regs(0x0041447e, call);
        uint16_t angle = static_cast<uint16_t>(call.eax);
        angle = static_cast<uint16_t>(angle - GP4_FIELD(uint16_t, car, 0x0e));
        angle = static_cast<uint16_t>(angle - 0x8000u);
        angle = static_cast<uint16_t>(0u - angle);
        eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(angle)));
    } else {
        const int32_t diff = static_cast<int16_t>(static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x0e) - static_cast<uint16_t>(ebp)));
        int32_t magnitude = diff < 0 ? -diff : diff;
        if (magnitude > 0x4000) {
            magnitude = 0x4000;
        }
        uint16_t index = static_cast<uint16_t>(magnitude);
        if (static_cast<int16_t>(index) < 0) {
            index = static_cast<uint16_t>(0u - index);
        }
        const int32_t cosine = GP4_ARRAY(int16_t, 0x007c5c5c)[index >> 3];
        eax = static_cast<uint32_t>(diff >= 0 ? -cosine : cosine);
    }

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
    if (static_cast<int32_t>(eax) < 0) {
        uint32_t mag = 0u - eax;
        if (mag >= limit) {
            mag = limit;
        }
        eax = 0u - mag;
    } else if (eax >= limit) {
        eax = limit;
    }

    GP4_FIELD(uint16_t, car, 0xb4) = static_cast<uint16_t>(eax);
    GP4_GLOBAL(uint8_t, 0x00603c34) = 0xffu;
    edx = eax;
    if (static_cast<int32_t>(edx) < 0) {
        edx = 0u - edx;
    }
    uint32_t flags = subFlags32(edx, 0x80u);
    if (static_cast<int32_t>(edx) >= 0x80) {
        GP4_FIELD(uint8_t, car, 0x6f) |= (static_cast<int16_t>(eax) < 0) ? 4u : 8u;
        flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x6f));
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u);
}
GP4_IMPL(0x0041416e, FUN_0041416e, "regs(eax:u32, edx:u32, ebp:u32, edi:rec[0x8943a0,0x160,0..29], esi:ptr[0x23d]:layout=car_0041416e+ptr_00414439) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_0041416e")
