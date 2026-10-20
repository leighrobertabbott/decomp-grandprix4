#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;
using gp4::x87::ext_i32;

struct Unknown_0041b600_Car {
    uint8_t Unknown00[0x6f];
    uint8_t Unknown6F;
    uint8_t Unknown70[0xee - 0x70];
    uint16_t UnknownEE;
    uint8_t UnknownF0[0xf6 - 0xf0];
    uint16_t UnknownF6;
    uint8_t UnknownF8[0x21c - 0xf8];
    uint8_t Unknown21C;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// (int64)x << 14 / divisor with the single IDIV of the original (cdq; shld edx,eax,14; shl eax,14).
static inline int32_t ratio14(int32_t x, int32_t divisor) {
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(static_cast<int64_t>(x)) << 14);
    int32_t remainder;
    return gp4::idiv64(dividend, divisor, &remainder);
}

// imul; shrd eax, edx, 14 - also returns the flags of that shrd (pinned oracle model: CF = last bit
// out, AF = 0, OF = bit 31 of (value >> 13) ^ (value >> 14)).
static inline uint32_t mulQ14(int32_t a, int32_t b, uint32_t* flags) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    *flags = (static_cast<uint32_t>(product >> 13) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    return result;
}

// Position [0x00623c34] >> 8, raised to [0x0062fb4c] when byte 0x00622424 is set and bit 0 of +0x6f.
static inline int32_t position(const Unknown_0041b600_Car* car) {
    int32_t value = GP4_GLOBAL(int32_t, 0x00623c34) >> 8;
    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0u && (car->Unknown6F & 1u) != 0u
        && value < GP4_GLOBAL(int32_t, 0x0062fb4c)) {
        value = GP4_GLOBAL(int32_t, 0x0062fb4c);
    }
    return value;
}

// FUNCTION: GP4 0x0041b600
// esi = car. DAT_006247a4 = max(0, ((pos - start) << 14) / span) with (start, span) chosen by bits
// 1/5 of +0x21c (0x006039e4/0x006039e8), byte 0x0062fb4a (0x0062fb7c/0x0062fb80, both offset by
// the span) or 0x00601314/0x00601318; DAT_00602bf4 = 0x4000 - min(that, 0x4000). The curve
// 0x00411891 (table 0x0062ee98) of pos, times 0x34d (byte 0x007c2ea9 == 0x80) or 0x357, / 0x358,
// times [0x006013f0], rounded and floored at [0x0062ee90], is multiplied in Q14 by DAT_006258c4
// (word +0xee, or the larger of it and the behind-ratio vs 0x00601328 / 0x006258a4), [0x00601e98],
// DAT_00602bf4, word 0x0062d204 and word +0xf6: eax = the product, flags of the last shrd.
// ebx, ecx and edx are saved and restored.
void __cdecl FUN_0041b600(gp4::Regs* r) {
    using namespace gp4::x87;
    const Unknown_0041b600_Car* car = reinterpret_cast<const Unknown_0041b600_Car*>(static_cast<uintptr_t>(r->esi));

    const int32_t pos = position(car);
    int32_t share;
    if ((car->Unknown21C & 2u) != 0u || (car->Unknown21C & 0x20u) != 0u) {
        const int32_t span = GP4_GLOBAL(int32_t, 0x006039e8);
        share = ratio14(static_cast<int32_t>(static_cast<uint32_t>(pos) - GP4_GLOBAL(uint32_t, 0x006039e4)
                                             + static_cast<uint32_t>(span)), span);
    } else if (GP4_GLOBAL(uint8_t, 0x0062fb4a) != 0u) {
        const int32_t span = GP4_GLOBAL(int32_t, 0x0062fb80);
        share = ratio14(static_cast<int32_t>(static_cast<uint32_t>(pos) - GP4_GLOBAL(uint32_t, 0x0062fb7c)
                                             + static_cast<uint32_t>(span)), span);
    } else {
        share = ratio14(static_cast<int32_t>(static_cast<uint32_t>(pos) - GP4_GLOBAL(uint32_t, 0x00601314)),
                        GP4_GLOBAL(int32_t, 0x00601318));
    }
    if (share < 0) {
        share = 0;
    }
    GP4_GLOBAL(int32_t, 0x006247a4) = share;
    if (share > 0x4000) {
        share = 0x4000;
    }
    GP4_GLOBAL(int32_t, 0x00602bf4) = 0x4000 - share;

    const int32_t pos2 = position(car);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = pos2;

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(pos2);
    call.ebx = 0x00603874u;
    call.edx = 0x0062ee98u;
    call.st_xin_mask = 0;
    call.st_xout_mask = 0;
    call.st_in = 1;
    call.st_out = 1;
    set_in_st(call, 0, ext_i32(pos2));
    gp4::call_regs(0x00411891, call);
    const Ext curve = out_st(call, 0);

    const int32_t numerator = GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u ? 0x34d : 0x357;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = numerator;
    const Ext product = curve * ext_i32(numerator);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = 0x358;
    const Ext scaled = (product / ext_i32(0x358)) * ext(GP4_GLOBAL(double, 0x006013f0));
    const int32_t rounded = fistp32(scaled);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;

    int32_t power = rounded;
    if (power < GP4_GLOBAL(int32_t, 0x0062ee90)) {
        power = GP4_GLOBAL(int32_t, 0x0062ee90);
    }

    int32_t factor = car->UnknownEE;
    const int32_t behind = (GP4_GLOBAL(int32_t, 0x00623c34) >> 8) - GP4_GLOBAL(int32_t, 0x00601328);
    if (behind < 0) {
        const int32_t distance = static_cast<int32_t>(0u - static_cast<uint32_t>(behind));
        if (distance < GP4_GLOBAL(int32_t, 0x006258a4)) {
            const int32_t q = ratio14(distance, GP4_GLOBAL(int32_t, 0x006258a4));
            if (factor < q) {
                factor = q;
            }
        } else {
            factor = 0x4000;
        }
    }
    GP4_GLOBAL(int32_t, 0x006258c4) = factor;

    uint32_t flags;
    uint32_t value = mulQ14(power, factor, &flags);
    value = mulQ14(static_cast<int32_t>(value), GP4_GLOBAL(int32_t, 0x00601e98), &flags);
    value = mulQ14(static_cast<int32_t>(value), GP4_GLOBAL(int32_t, 0x00602bf4), &flags);
    value = mulQ14(static_cast<int32_t>(value), static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x0062d204)), &flags);
    value = mulQ14(static_cast<int32_t>(value), static_cast<int32_t>(car->UnknownF6), &flags);

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b600, FUN_0041b600, "regs(esi:ptr[0x228]:bytes) -> (eax, cf, pf, af, zf, sf, of); cases=200; globals=fuzz:i32+layout=state_0041b950+state_00411109+state_0041b600")
