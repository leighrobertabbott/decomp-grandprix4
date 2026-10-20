#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte-wide logic result (test / and / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 32-bit `left - right` (sub / cmp).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// High dword (edx) of the signed 64-bit product.
static inline uint32_t mulHigh(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

// |sign-extended 16-bit value|, as `movsx eax, ax ; test ; jns ; neg eax`.
static inline int32_t absSext16(uint16_t value) {
    int32_t v = static_cast<int16_t>(value);
    if (v < 0) {
        v = static_cast<int32_t>(0u - static_cast<uint32_t>(v));
    }
    return v;
}

// Track segment record addressed by the pointer global 0x00624868.
struct Unknown_00427d4b_Segment {
    int16_t Unknown00;
    uint8_t Unknown02[0x18 - 0x02];
    int16_t Unknown18;
    uint8_t Unknown1A[0x51 - 0x1a];
    uint8_t Flags51;
};

// Fields of the car record (esi) this routine reads.
struct Unknown_00427d4b_Car {
    uint8_t Unknown00[0x18];
    int16_t Unknown18;
    uint8_t Unknown1A[0x71 - 0x1a];
    uint8_t Flags71;
    uint8_t Unknown72[0x90 - 0x72];
    int16_t Unknown90;
    uint8_t Unknown92[0x94 - 0x92];
    int16_t Unknown94;
    uint8_t Unknown96[0x9c - 0x96];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xb0 - 0x9d];
    int16_t UnknownB0;
    uint8_t UnknownB2[0xfe - 0xb2];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x122 - 0xff];
    uint8_t Flags122;
    uint8_t Unknown123[0x163 - 0x123];
    uint8_t Flags163;
    uint8_t Unknown164[0x1a0 - 0x164];
    int32_t Unknown1A0;
};

// True when one of the exclusion tests sends the original to the plain-value exit; `flags` is left
// with the flags of the last test executed.
static inline bool takesPlainValue(const Unknown_00427d4b_Car* car, uint32_t& flags) {
    uint32_t t = car->FlagsFE & 0x40u;
    flags = logicFlags8(t);
    if (t != 0u) {
        return true;
    }
    t = car->Flags9C & 0x10u;
    flags = logicFlags8(t);
    if (t != 0u) {
        return true;
    }
    t = car->Flags163 & 0x80u;
    flags = logicFlags8(t);
    if (t != 0u) {
        t = car->Flags122 & 0x80u;
        flags = logicFlags8(t);
        if (t != 0u) {
            return true;
        }
        t = car->Flags163 & 0x40u;
        flags = logicFlags8(t);
        if (t == 0u) {
            return true;
        }
    }
    const Unknown_00427d4b_Segment* seg = GP4_GLOBAL(const Unknown_00427d4b_Segment*, 0x00624868);
    t = seg->Flags51 & 0x20u;
    flags = logicFlags8(t);
    if (t != 0u) {
        const bool segBit3 = (seg->Flags51 & 8u) != 0u;
        t = car->Flags71 & 8u;
        flags = logicFlags8(t);
        if (segBit3 ? (t == 0u) : (t != 0u)) {
            return true;
        }
    }
    return false;
}

// FUNCTION: GP4 0x00427d4b
// Computes the scale stored in [0x624858] (Q14, 0x4000 = 1.0). Unless the car is excluded (bits in
// +0xfe, +0x9c, +0x163, +0x122, or +0x71 against the segment's +0x51), the value is
// +0x1a0 * (a * b) + 0x4000 - a * b where a = min(0x4000, max(|word +0x94 - word +0x90| * [0x624874],
// |seg[0] + seg[0x18] - word +0x18| * [0x624878])) and b = clamp(|+/-word +0xb0 - word +0x94| *
// [0x62487c], 0, 0x4000), every product shifted right by 14. Otherwise it is dword +0x1a0. In the
// computed case ebp holds a*b and edx the high dword of the last product; edi is restored.
void __cdecl FUN_00427d4b(gp4::Regs* r) {
    const Unknown_00427d4b_Car* car = reinterpret_cast<const Unknown_00427d4b_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;
    uint32_t flags;

    if (takesPlainValue(car, flags)) {
        eax = static_cast<uint32_t>(car->Unknown1A0);
    } else {
        const Unknown_00427d4b_Segment* seg = GP4_GLOBAL(const Unknown_00427d4b_Segment*, 0x00624868);

        const int32_t speedTerm = static_cast<int32_t>(mulShr14(
            absSext16(static_cast<uint16_t>(static_cast<uint16_t>(car->Unknown94) - static_cast<uint16_t>(car->Unknown90))),
            GP4_GLOBAL(int32_t, 0x00624874)));

        const int32_t lateralTerm = static_cast<int32_t>(mulShr14(
            absSext16(static_cast<uint16_t>(
                static_cast<uint16_t>(static_cast<uint16_t>(seg->Unknown00) + static_cast<uint16_t>(seg->Unknown18))
                - static_cast<uint16_t>(car->Unknown18))),
            GP4_GLOBAL(int32_t, 0x00624878)));

        int32_t a = lateralTerm < speedTerm ? speedTerm : lateralTerm;
        if (a > 0x4000) {
            a = 0x4000;
        }

        uint16_t bw = static_cast<uint16_t>(car->UnknownB0);
        if (car->Unknown90 < 0) {
            bw = static_cast<uint16_t>(0u - bw);
        }
        bw = static_cast<uint16_t>(bw - static_cast<uint16_t>(car->Unknown94));
        int32_t b = static_cast<int32_t>(mulShr14(absSext16(bw), GP4_GLOBAL(int32_t, 0x0062487c)));
        if (b < 0) {
            b = 0;
        }
        if (b > 0x4000) {
            b = 0x4000;
        }

        ebp = mulShr14(b, a);
        const uint32_t scaled = mulShr14(static_cast<int32_t>(ebp), car->Unknown1A0);
        edx = mulHigh(static_cast<int32_t>(ebp), car->Unknown1A0);
        const uint32_t sum = scaled + 0x4000u;
        flags = subFlags32(sum, ebp);
        eax = sum - ebp;
    }

    GP4_GLOBAL(int32_t, 0x00624858) = static_cast<int32_t>(eax);
    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00427d4b, FUN_00427d4b, "regs(esi:ptr[0x1a8]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00427d4b")
