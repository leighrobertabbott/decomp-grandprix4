#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 16-bit logic result (test word, 0xffff): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffffu) == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `left - right` (cmp / neg).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (a * b) >> 14: low dword of the shifted product ("imul; shrd eax, edx, 14"), *high = edx.
static inline uint32_t mulShift14(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0041b286
// Piecewise response curve of the value eax (|eax| against the thresholds 0x602948 / 0x60294c, which
// it first recomputes): the factor 0x601264 = max(0x4000 - (max(|word [[esi+0x1c]+0x26]| - [0x60293c],
// 0) * [0x602940] >> 14), [0x602944]) scales [0x602930] / [0x602934] / [0x602938] (>> 14) into
// 0x602948 / 0x60294c / 0x602950. |eax| below 0x602948 returns eax unchanged; between that and 0x60294c
// the result is cos-table((0x4000 - ((|eax| - t0) << 14) / (t1 - t0))) * t2 >> 14 + t0; above, it is
// (0x4000 - ((min(|eax|, 0x4000) - t1) << 14) / (0x4000 - t1)) * (t2 + t0) >> 14. Both are negated when
// the word 0x679c1c is negative. ebp = the divisor (or the factor), edx = the high dword of the last
// product (|eax| when unchanged); the flags are those of the last compare / neg.
void __cdecl FUN_0041b286(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, car, 0x1c)));

    int32_t excess = GP4_FIELD(int16_t, segment, 0x26);
    if (excess < 0) {
        excess = -excess;
    }
    excess = static_cast<int32_t>(static_cast<uint32_t>(excess) - GP4_GLOBAL(uint32_t, 0x0060293c));
    if (excess < 0) {
        excess = 0;
    }
    uint32_t high;
    int32_t factor = static_cast<int32_t>(0x4000u - mulShift14(excess, GP4_GLOBAL(int32_t, 0x00602940), &high));
    if (factor < GP4_GLOBAL(int32_t, 0x00602944)) {
        factor = GP4_GLOBAL(int32_t, 0x00602944);
    }
    GP4_GLOBAL(int32_t, 0x00601264) = factor;
    const uint32_t lower = mulShift14(GP4_GLOBAL(int32_t, 0x00602930), factor, &high);
    GP4_GLOBAL(uint32_t, 0x00602948) = lower;
    const uint32_t upper = mulShift14(GP4_GLOBAL(int32_t, 0x00602934), factor, &high);
    GP4_GLOBAL(uint32_t, 0x0060294c) = upper;
    const uint32_t peak = mulShift14(GP4_GLOBAL(int32_t, 0x00602938), factor, &high);
    GP4_GLOBAL(uint32_t, 0x00602950) = peak;

    uint32_t edx = r->eax;
    if (static_cast<int32_t>(edx) < 0) {
        edx = 0u - edx;
    }
    uint32_t flags = subFlags32(edx, lower);
    if (static_cast<int32_t>(edx) < static_cast<int32_t>(lower)) {
        r->edx = edx;
        r->ebp = static_cast<uint32_t>(factor);
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    uint32_t eax = edx;
    uint32_t divisor;
    int32_t remainder;
    if (static_cast<int32_t>(eax) > static_cast<int32_t>(upper)) {
        if (static_cast<int32_t>(eax) > 0x4000) {
            eax = 0x4000u;
        }
        eax -= upper;
        divisor = 0x4000u - upper;
        const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(eax)) << 14, static_cast<int32_t>(divisor), &remainder);
        eax = 0x4000u - static_cast<uint32_t>(quotient);
        eax = mulShift14(static_cast<int32_t>(eax), static_cast<int32_t>(peak + lower), &edx);
    } else {
        eax -= lower;
        divisor = upper - lower;
        if (divisor == 0u) {
            divisor = 1u;
        }
        const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(eax)) << 14, static_cast<int32_t>(divisor), &remainder);
        uint16_t angle = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(quotient));
        if (static_cast<int16_t>(angle) < 0) {
            angle = static_cast<uint16_t>(0u - angle);
        }
        const int32_t cosine = GP4_ARRAY(int16_t, 0x007c5c5c)[angle >> 3];
        eax = mulShift14(cosine, static_cast<int32_t>(peak), &edx) + lower;
    }

    const uint16_t sign = GP4_GLOBAL(uint16_t, 0x00679c1c);
    flags = logicFlags16(sign);
    if (static_cast<int16_t>(sign) < 0) {
        flags = subFlags32(0u, eax);
        eax = 0u - eax;
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b286, FUN_0041b286, "regs(eax:i32, esi:ptr[0x40]:layout=car_0042ba87) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
