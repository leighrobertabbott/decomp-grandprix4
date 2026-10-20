#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `cmp left, right` / `sub left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of a byte `test reg/mem, imm` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

static inline uint32_t setLow16(uint32_t reg, uint32_t value) {
    return (reg & 0xffff0000u) | (value & 0xffffu);
}

struct Unknown_00434700_Car {
    uint8_t Unknown00[0xe];
    uint16_t UnknownE;
    uint8_t Unknown10[0x7b - 0x10];
    uint8_t Unknown7B;
    uint8_t Unknown7C[0x90 - 0x7c];
    int16_t Unknown90;
    uint8_t Unknown92[0x9a - 0x92];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xac - 0x9c];
    int32_t UnknownAC;
    int16_t UnknownB0;
    uint8_t UnknownB2[0xe5 - 0xb2];
    uint8_t UnknownE5;
    uint8_t UnknownE6[0xfe - 0xe6];
    uint8_t UnknownFE;
    uint8_t UnknownFF[0x26b - 0xff];
    uint8_t Unknown26B;
};

struct Unknown_00434700_Target {
    uint16_t Unknown00;
    uint8_t Unknown02[0x18 - 0x02];
    uint16_t Unknown18;
};

// Clamps the signed word `value` to +-DAT_0060381c (the magnitude compare is unsigned), scales it by
// the signed word DAT_0060384c (imul word: the high half is kept) and subtracts that from word +0xe.
// ax and the low half of edx hold the high half afterwards; the flags are those of the final sub.
static inline void applyCorrection(Unknown_00434700_Car* car, uint16_t value, uint32_t* eax, uint32_t* edx, uint32_t* flags) {
    const uint16_t limit = GP4_GLOBAL(uint16_t, 0x0060381c);
    uint16_t v = value;
    if (static_cast<int16_t>(v) < 0) {
        v = static_cast<uint16_t>(0u - v);
        if (!(v < limit)) {
            v = limit;
        }
        v = static_cast<uint16_t>(0u - v);
    } else if (!(v < limit)) {
        v = limit;
    }
    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(v)) * static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x0060384c));
    const uint32_t high = (static_cast<uint32_t>(product) >> 16) & 0xffffu;
    *eax = setLow16(*eax, high);
    *edx = setLow16(*edx, high);
    const uint16_t before = car->UnknownE;
    car->UnknownE = static_cast<uint16_t>(before - high);
    *flags = cmpFlags16(before, high);
}

// FUNCTION: GP4 0x00434700
// esi = car record, edi = the car's reference word pair ([edi] and [edi+0x18]). The lateral error
// D = word +0xe - [edi] - [edi+0x18] is pulled back: while the correction flag (bit 0 of +0xfe) is set
// it is clamped by +-DAT_0060381c, scaled by DAT_0060384c (>> 16) and subtracted from +0xe (the flag
// clears once |D| < DAT_00603820); otherwise, for cars below DAT_006013e4 in +0x9a with |D| >= 0x800
// and enough error plus steering term (+0x90, +0xb0), the flag is raised and the same correction is
// applied. eax / ecx / edx are changed in their low words (edx wholly after the +0xac test); the flags are
// those of the last compare / test / sub.
void __cdecl FUN_00434700(gp4::Regs* r) {
    Unknown_00434700_Car* car = reinterpret_cast<Unknown_00434700_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00434700_Target* target = reinterpret_cast<const Unknown_00434700_Target*>(static_cast<uintptr_t>(r->edi));
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t flags;

    flags = testFlags8(car->Unknown26B & 0x80u);
    if ((car->Unknown26B & 0x80u) != 0u) {
        goto done;
    }
    flags = testFlags8(car->Unknown7B & 0x80u);
    if ((car->Unknown7B & 0x80u) != 0u) {
        goto done;
    }

    if ((car->UnknownFE & 1u) != 0u) {
        const uint16_t error = static_cast<uint16_t>(car->UnknownE - target->Unknown00 - target->Unknown18);
        eax = setLow16(eax, error);
        uint16_t magnitude = error;
        if (static_cast<int16_t>(magnitude) < 0) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        edx = setLow16(edx, magnitude);
        if (!(static_cast<int16_t>(magnitude) >= GP4_GLOBAL(int16_t, 0x00603820))) {
            car->UnknownFE = static_cast<uint8_t>(car->UnknownFE & 0xfeu);
        }
        applyCorrection(car, error, &eax, &edx, &flags);
        goto done;
    }

    flags = testFlags8(car->UnknownE5 & 0x80u);
    if ((car->UnknownE5 & 0x80u) != 0u) {
        goto done;
    }
    {
        const int16_t level = car->Unknown9A;
        eax = setLow16(eax, static_cast<uint16_t>(level));
        flags = cmpFlags16(static_cast<uint16_t>(level), GP4_GLOBAL(uint16_t, 0x006013e4));
        if (level >= GP4_GLOBAL(int16_t, 0x006013e4)) {
            goto done;
        }
    }

    {
        const uint16_t error = static_cast<uint16_t>(car->UnknownE - target->Unknown00 - target->Unknown18);
        ecx = setLow16(ecx, error);
        uint16_t magnitude = error;
        if (static_cast<int16_t>(magnitude) < 0) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        edx = setLow16(edx, magnitude);
        flags = cmpFlags16(magnitude, 0x800u);
        if (static_cast<int16_t>(magnitude) < 0x800) {
            goto done;
        }

        int16_t steer = car->Unknown90;
        if (steer < 0) {
            steer = static_cast<int16_t>(0 - steer);
        }
        steer = static_cast<int16_t>(steer - car->UnknownB0);
        if (steer < 0) {
            steer = 0;
        }
        if (steer > 0x200) {
            steer = 0x200;
        }
        if (car->Unknown90 < 0) {
            steer = static_cast<int16_t>(0 - steer);
        }
        const uint16_t adjusted = static_cast<uint16_t>(static_cast<uint16_t>(steer << 3) + error);
        eax = setLow16(eax, adjusted);
        magnitude = adjusted;
        if (static_cast<int16_t>(magnitude) < 0) {
            magnitude = static_cast<uint16_t>(0u - magnitude);
        }
        edx = setLow16(edx, magnitude);
        flags = cmpFlags16(magnitude, GP4_GLOBAL(uint16_t, 0x00603814));
        if (static_cast<int16_t>(magnitude) < GP4_GLOBAL(int16_t, 0x00603814)) {
            goto done;
        }

        edx = static_cast<uint32_t>(car->UnknownAC >> 11);
        flags = cmpFlags16(edx, GP4_GLOBAL(uint16_t, 0x00603818));
        if (static_cast<int16_t>(edx) >= GP4_GLOBAL(int16_t, 0x00603818)) {
            goto done;
        }
        car->UnknownFE = static_cast<uint8_t>(car->UnknownFE | 1u);
        applyCorrection(car, adjusted, &eax, &edx, &flags);
    }

done:
    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434700, FUN_00434700, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x273]:layout=car_00434700, edi:ptr[0x22]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); cases=400; globals=fuzz:i32+layout=lim_00434700")
