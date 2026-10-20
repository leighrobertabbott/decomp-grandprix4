#include <gp4/gp4.h>

// esi: a car record (only the fields read here are named).
struct Unknown_0044192c_Car {
    uint8_t Unknown00[0x7f];
    uint8_t Unknown7F;              // driver level / table class, 0..5
    uint8_t Unknown80[0x240 - 0x80];
    uint32_t Unknown240;            // four packed position bytes
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

// FUNCTION: GP4 0x0044192c
// Force/torque curve of one car: eax is a scaled input and ecx selects which byte of the packed position dword
// +0x240 is the curve position. A1 = curve 0x00602dec at that position (0x0041efb5, record
// [0x0062d1d8 + 4 * level]), B1 = curve 0x00602c44 (0x0041eff0, record = level); then
// x = ((B1 * eax) >> 14) * word [0x0062d236] >> 28, above 0x280 stretched by 0x00441792 and the two tables
// 0x0062d25c / 0x0062d258. x is looked up in the breakpoint pairs at 0x00602f74 (terminated by -1) and linearly
// interpolated (floor [0x00602c24 + 4 * record]); the result is that value plus (A1 * word [0x0062d1fa]) >> 14,
// or only the latter when [0x007c2e75] is nonzero. Every register but eax is restored; the flags are those of
// the final add (or of the compare of [0x007c2e75] with zero).
void __cdecl FUN_0044192c(gp4::Regs* r) {
    const Unknown_0044192c_Car* car = reinterpret_cast<const Unknown_0044192c_Car*>(static_cast<uintptr_t>(r->esi));
    const int32_t input = static_cast<int32_t>(r->eax);
    const uint32_t level = car->Unknown7F;
    const uint32_t record = GP4_ARRAY(const uint32_t, 0x0062d1d8)[level];

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.ebp = r->eax;
    c.ebx = record;
    c.edx = (car->Unknown240 >> ((r->ecx << 3) & 31u)) & 0xffu;
    gp4::call_regs(0x0041efb5, c);
    const uint32_t curveA = c.eax;

    c.ebx = level;
    gp4::call_regs(0x0041eff0, c);
    const uint32_t curveB = c.eax;

    uint32_t value = mulShift14(static_cast<int32_t>(curveB), input);
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
        uint32_t span = breakpoint[0] - startX;
        if (span == 0u) {
            span = 1u;
        }
        const int32_t rise = static_cast<int32_t>(breakpoint[1] - startY);
        const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(value - startX)) * static_cast<int64_t>(rise);
        int32_t remainder;
        const int32_t step = gp4::idiv64(numerator, static_cast<int32_t>(span), &remainder);
        curve = startY + static_cast<uint32_t>(step);
        if (static_cast<int32_t>(curve) < floorValue) {
            curve = static_cast<uint32_t>(floorValue);
        }
    }

    const uint32_t extra = mulShift14(static_cast<int32_t>(curveA),
                                      static_cast<int32_t>(static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x0062d1fa))));
    const uint8_t replaceMode = GP4_GLOBAL(uint8_t, 0x007c2e75);
    uint32_t flags;
    if (replaceMode == 0u) {
        flags = addFlags32(curve, extra);
        curve += extra;
    } else {
        flags = cmpFlags8(replaceMode, 0u);
        curve = extra;
    }

    r->eax = curve;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044192c, FUN_0044192c, "regs(eax:i32[-0x10000..0x1000000], ecx:u32[0..3], esi:ptr[0x248]:layout=rec_0043259c) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_004320f2")
