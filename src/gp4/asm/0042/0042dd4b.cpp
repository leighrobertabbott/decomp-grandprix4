#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of `cmp eax, 0`: CF = OF = AF = 0.
static inline uint32_t cmpZeroFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// Speed word +0x9a of a car, capped at 0x1000, times 4.
static inline int32_t scaledSpeed(uint8_t* car) {
    int32_t speed = GP4_FIELD(int16_t, car, 0x9a);
    if (speed > 0x1000) {
        speed = 0x1000;
    }
    return speed << 2;
}

// Angle word +0x18 minus +0x142 of a car, folded into -0x4000..0x4000 (a difference beyond a quarter
// turn is mirrored about it), sign extended.
static inline int32_t foldedAngle(uint8_t* car) {
    uint16_t angle = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x18) - GP4_FIELD(uint16_t, car, 0x142));
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
        if (angle > 0x4000u) {
            angle = static_cast<uint16_t>((0u - angle) + 0x8000u);
        }
        angle = static_cast<uint16_t>(0u - angle);
    } else if (angle > 0x4000u) {
        angle = static_cast<uint16_t>((0u - angle) + 0x8000u);
    }
    return static_cast<int16_t>(angle);
}

static inline void finish(gp4::Regs* r, uint32_t eax, uint32_t flags) {
    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x0042dd4b
// Car pair esi / ebx (ebx, ecx, edx, ebp, esi, edi are saved and restored). The gap word [0x603e70] is
// the +0xe2 difference of the two cars (or 0x0040dc8a when either has bit 1 of +0x9c). CF = 1 (stc) when
// the gap is below the word [0x603b56], when ebx (bit 1 of +0x8d, neither car with bit 4 of +0xce) and
// the words +0xea = 0x18 (esi) / 0x20 (ebx) rule the pair out, or when 0x0042beea fails. Otherwise
// 0x0042b0d5 sets [0x603e5e] and eax = ((P(esi) - P(ebx)) * [0x603c60] >> 14) (0 when byte [0x603e6e] is
// negative), P = (min(+0x9a, 0x1000) * 4) * folded(+0x18 - +0x142) >> 14, plus the word [0x604404], moved
// [0x603e5e] towards zero and clamped there (sign extended). CF = 0 (clc); the other flags are those of
// the last cmp / test.
void __cdecl FUN_0042dd4b(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    uint8_t* self = bytesAt(r->esi);
    uint8_t* other = bytesAt(r->ebx);

    uint16_t gap;
    if ((GP4_FIELD(uint8_t, self, 0x9c) & 2u) == 0u && (GP4_FIELD(uint8_t, other, 0x9c) & 2u) == 0u) {
        gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, other, 0xe2) - GP4_FIELD(uint16_t, self, 0xe2));
        s.eax = (s.eax & 0xffff0000u) | gap;
    } else {
        gp4::call_regs(0x0040dc8a, s);
        gap = static_cast<uint16_t>(s.eax);
    }
    GP4_GLOBAL(uint16_t, 0x00603e70) = gap;

    const uint16_t floor = GP4_GLOBAL(uint16_t, 0x00603b56);
    if (static_cast<int16_t>(gap) < static_cast<int16_t>(floor)) {
        finish(r, s.eax, (subFlags16(gap, floor) & ~1u) | 1u);
        return;
    }

    if ((GP4_FIELD(uint8_t, other, 0x8d) & 2u) != 0u && (GP4_FIELD(uint8_t, self, 0xce) & 0x10u) == 0u
        && (GP4_FIELD(uint8_t, other, 0xce) & 0x10u) == 0u && GP4_FIELD(uint16_t, self, 0xea) == 0x18u
        && GP4_FIELD(uint16_t, other, 0xea) == 0x20u) {
        finish(r, s.eax, 0x44u | 1u);  // cmp word [ebx+0xea], 0x20: equal; stc
        return;
    }

    gp4::call_regs(0x0042beea, s);
    if ((s.eflags & 1u) != 0u) {
        finish(r, s.eax, (s.eflags & 0x8d4u) | 1u);
        return;
    }
    gp4::call_regs(0x0042b0d5, s);

    uint32_t steer = 0;
    if ((GP4_GLOBAL(uint8_t, 0x00603e6e) & 0x80u) == 0u) {
        const uint32_t behind = mulShr14(scaledSpeed(other), foldedAngle(other));
        const uint32_t ahead = mulShr14(scaledSpeed(self), foldedAngle(self));
        steer = mulShr14(static_cast<int32_t>(ahead - behind), GP4_GLOBAL(int32_t, 0x00603c60));
    }

    const uint16_t total = static_cast<uint16_t>(static_cast<uint16_t>(steer) + GP4_GLOBAL(uint16_t, 0x00604404));
    int32_t result;
    uint32_t flags;
    if (static_cast<int16_t>(total) < 0) {
        result = static_cast<int16_t>(static_cast<uint16_t>(total + GP4_GLOBAL(uint16_t, 0x00603e5e)));
        flags = cmpZeroFlags32(static_cast<uint32_t>(result));
        if (result > 0) {
            result = 0;
        }
    } else {
        result = static_cast<int16_t>(static_cast<uint16_t>(total - GP4_GLOBAL(uint16_t, 0x00603e5e)));
        flags = cmpZeroFlags32(static_cast<uint32_t>(result));
        if (result < 0) {
            result = 0;
        }
    }
    finish(r, static_cast<uint32_t>(result), flags);
}
GP4_IMPL(0x0042dd4b, FUN_0042dd4b, "regs(eax:u32, ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042dd4b, ecx:u32, edx:u32, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+pos_00405cf8+car_0042b17e+car_0043c677+car_0042dd4b, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd")
