#include <gp4/gp4.h>

// esi: a car record (only the fields read here are named).
struct Unknown_0044182d_Car {
    uint8_t Unknown00[0x7c];
    uint8_t Unknown7C;              // car number (low 6 bits), 1-based
    uint8_t Unknown7D[2];
    uint8_t Unknown7F;              // driver level / table class, 0..5
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One-operand `imul` then `shrd eax, edx, 14`: eax = bits 14..45 of the signed 64-bit product.
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    return static_cast<uint32_t>(product >> 14);
}

// FUNCTION: GP4 0x0044182d
// Force/torque curve of one car (the variant of 0x0044192c that takes the position from 0x00441758 and a per-car
// scale from the 22 x 0x2c dword table 0x0066ec60): A1 = curve 0x00602dec at the position (0x0041efb5, record
// [0x0062d1d8 + 4 * level]), B1 = curve 0x00602c44 (0x0041eff0, record = level); x = ((B1 * scale) >> 14) *
// word [0x0062d236] >> 28, above 0x280 stretched by 0x00441792 and the tables 0x0062d25c / 0x0062d258. x is looked
// up in the breakpoint pairs at 0x00602f74 (terminated by -1) and linearly interpolated (floor [0x00602c24 + 4 *
// record]); ecx is that value plus (A1 * word [0x0062d1fa]) >> 14 (eax = that second term, edx = its high half),
// or only the latter when [0x007c2e75] is nonzero. ebx is the curve record, ebp the last divisor (or the scale);
// edi is restored; the flags are those of the final add (or of the compare of [0x007c2e75] with zero).
void __cdecl FUN_0044182d(gp4::Regs* r) {
    const Unknown_0044182d_Car* car = reinterpret_cast<const Unknown_0044182d_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00441758, c);
    const uint32_t position = c.eax;

    int32_t carIndex = static_cast<int32_t>(car->Unknown7C & 0x3fu) - 1;
    if (carIndex > 0x15) {
        carIndex = 0x15;
    }
    const uint32_t pick = GP4_ARRAY(const uint8_t, 0x0066d936)[carIndex];
    uint32_t ebp = GP4_ARRAY(const uint32_t, 0x0066ec60)[carIndex * 0x2c + static_cast<int32_t>(pick)];

    const uint32_t level = car->Unknown7F;
    const uint32_t record = GP4_ARRAY(const uint32_t, 0x0062d1d8)[level];

    c.edx = position;
    c.ebx = record;
    gp4::call_regs(0x0041efb5, c);
    const uint32_t curveA = c.eax;

    c.ebx = level;
    gp4::call_regs(0x0041eff0, c);
    const uint32_t curveB = c.eax;

    uint32_t value = mulShift14(static_cast<int32_t>(curveB), static_cast<int32_t>(ebp));
    const uint64_t scaled = static_cast<uint64_t>(value) * static_cast<uint64_t>(GP4_GLOBAL(uint16_t, 0x0062d236));
    value = static_cast<uint32_t>(scaled >> 28);

    if (static_cast<int32_t>(value) >= 0x280) {
        c.eax = value - 0x280u;
        c.edx = level;
        gp4::call_regs(0x00441792, c);
        value = mulShift14(static_cast<int32_t>(c.eax), static_cast<int32_t>(GP4_ARRAY(const uint32_t, 0x0062d25c)[level]));
        value = mulShift14(static_cast<int32_t>(value), static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062d258)));
        value += 0x280u;
    }

    const uint32_t* breakpoint = &GP4_ARRAY(const uint32_t, 0x00602f74)[0];
    uint32_t curve;
    bool found = false;
    for (;;) {
        if (static_cast<int32_t>(value) < static_cast<int32_t>(breakpoint[0])) {
            found = true;
            break;
        }
        breakpoint += 2;
        if (breakpoint[0] == 0xffffffffu) {
            break;
        }
    }

    const int32_t floorValue = static_cast<int32_t>(GP4_ARRAY(const uint32_t, 0x00602c24)[record]);
    if (!found) {
        curve = static_cast<uint32_t>(floorValue);
    } else {
        const uint32_t startX = breakpoint[-2];
        const uint32_t startY = breakpoint[-1];
        ebp = breakpoint[0] - startX;
        if (ebp == 0u) {
            ebp = 1u;
        }
        const int32_t rise = static_cast<int32_t>(breakpoint[1] - startY);
        const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(value - startX)) * static_cast<int64_t>(rise);
        int32_t remainder;
        const int32_t step = gp4::idiv64(numerator, static_cast<int32_t>(ebp), &remainder);
        curve = startY + static_cast<uint32_t>(step);
        if (static_cast<int32_t>(curve) < floorValue) {
            curve = static_cast<uint32_t>(floorValue);
        }
    }

    const int64_t extraProduct = static_cast<int64_t>(static_cast<int32_t>(curveA))
        * static_cast<int64_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x0062d1fa)));
    const uint32_t extra = static_cast<uint32_t>(static_cast<uint64_t>(extraProduct) >> 14);
    const uint32_t extraHigh = static_cast<uint32_t>(static_cast<uint64_t>(extraProduct) >> 32);
    const uint8_t replaceMode = GP4_GLOBAL(uint8_t, 0x007c2e75);
    uint32_t flags;
    if (replaceMode == 0u) {
        flags = addFlags32(curve, extra);
        curve += extra;
    } else {
        flags = cmpFlags8(replaceMode, 0u);
        curve = extra;
    }

    r->eax = extra;
    r->ebx = record;
    r->ecx = curve;
    r->edx = extraHigh;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044182d, FUN_0044182d, "regs(esi:ptr[0x248]:layout=rec_0043259c) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_004320f2+counter_0040f0dd+scale_0066ec60")
