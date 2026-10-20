#include <gp4/gp4.h>

// low 32 bits of (a * b) >> n, as imul + shrd eax, edx, n.
static inline int32_t mulShift8(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 8));
}
static inline int32_t mulShift14(int32_t a, int32_t b) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14));
}

// Q14 table word at 4 * (|angle| >> 4) words (shr ax, 2; and 0xfffe), angle a signed 16-bit word.
static inline int32_t tableAt(uint16_t angle) {
    if ((angle & 0x8000u) != 0u) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t index = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), index);
}

// Angle word [0x629dd2] - word +0xe of the record in esi, offset by +-0x4000 by the sign of word
// +0x90, selects the first of 20 thresholds (0x629f44) above it (else entry 0). With that entry's
// reference word (0x629f6c) and gain (0x629f94) it forms two Q14 table products, scaled by
// |[0x629dce]| and [0x629e8c], and by [0x629dd0], [0x629e88] and min(|[0x629dce]| * [0x629e74] >>
// 14, 0x4000); their difference, clamped in magnitude to [0x629e7c], times [0x629e78] << 9 is
// divided by [0x629e6c] and subtracted from dword +0xac; times [0x629e6c] >> 14 it goes to +0x158
// and negated to +0xac. Then it sets the vector 0x629dd4/d8/dc and calls 0x4389a5 with the entry's
// mask (0x629fe4) while byte 0x629f30 is 0xff and 0x629f40 holds 0x629f38.
// FUNCTION: GP4 0x00437d47
void __cdecl FUN_00437d47(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint16_t offset = (GP4_FIELD(int16_t, record, 0x90) < 0) ? 0xc000u : 0x4000u;
    const uint16_t angle = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00629dd2) - GP4_FIELD(uint16_t, record, 0x0e) + offset);
    GP4_GLOBAL(uint16_t, 0x00629f2c) = angle;

    const uint16_t* thresholds = GP4_ARRAY(uint16_t, 0x00629f44);
    uint32_t entry = 0;
    while (!(angle < thresholds[entry])) {
        ++entry;
        if (entry == 0x14u) {
            entry = 0;
            break;
        }
    }

    const uint16_t reference = GP4_ARRAY(uint16_t, 0x00629f6c)[entry];
    const int32_t gain = GP4_ARRAY(int32_t, 0x00629f94)[entry];
    const uint16_t delta = static_cast<uint16_t>(reference - GP4_GLOBAL(uint16_t, 0x00629f2c));

    int32_t speed = GP4_GLOBAL(int16_t, 0x00629dce);
    if (speed < 0) {
        speed = -speed;
    }
    int32_t first = mulShift8(tableAt(static_cast<uint16_t>(0x4000u - delta)), gain);
    first = mulShift14(first, speed);
    first = mulShift14(first, GP4_GLOBAL(int32_t, 0x00629e8c));

    int32_t second = mulShift8(tableAt(delta), gain);
    second = mulShift14(second, GP4_GLOBAL(int16_t, 0x00629dd0));
    second = mulShift14(second, GP4_GLOBAL(int32_t, 0x00629e88));

    int32_t blend = mulShift14(speed, GP4_GLOBAL(int32_t, 0x00629e74));
    if (blend > 0x4000) {
        blend = 0x4000;
    }
    second = mulShift14(second, blend);

    uint32_t value = static_cast<uint32_t>(first) - static_cast<uint32_t>(second);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00629e7c);
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (!(magnitude < limit)) {
            magnitude = limit;
        }
        value = 0u - magnitude;
    } else if (!(value < limit)) {
        value = limit;
    }

    const int32_t divisor = GP4_GLOBAL(int32_t, 0x00629e6c);
    const uint64_t scaled = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(value))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00629e78))) << 9;
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(scaled), divisor, &remainder);
    const int32_t adjusted = static_cast<int32_t>(GP4_FIELD(uint32_t, record, 0xac) - static_cast<uint32_t>(quotient));
    const int32_t result = mulShift14(adjusted, divisor);
    GP4_FIELD(int32_t, record, 0x158) = result;
    GP4_FIELD(uint32_t, record, 0xac) = 0u - static_cast<uint32_t>(result);

    GP4_GLOBAL(uint32_t, 0x00629dd4) = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00629dce))) << 16;
    GP4_GLOBAL(uint32_t, 0x00629dd8) = 0;
    const uint32_t lift = static_cast<uint32_t>(mulShift14(GP4_GLOBAL(int16_t, 0x00629dd0), GP4_GLOBAL(int32_t, 0x00629e80))) << 16;
    const int64_t liftProduct = static_cast<int64_t>(static_cast<int32_t>(lift)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00629f34));
    GP4_GLOBAL(uint32_t, 0x00629ddc) = static_cast<uint32_t>(static_cast<uint64_t>(liftProduct) >> 14);
    GP4_GLOBAL(uint8_t, 0x00629f30) = 0xff;

    const uint32_t saved = GP4_GLOBAL(uint32_t, 0x00629f40);
    GP4_GLOBAL(uint32_t, 0x00629f40) = GP4_GLOBAL(uint32_t, 0x00629f38);
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = GP4_ARRAY(uint32_t, 0x00629fe4)[entry];
    call.ebx = entry;
    call.ecx = static_cast<uint32_t>(divisor);
    call.edx = static_cast<uint32_t>(static_cast<uint64_t>(liftProduct) >> 32);
    gp4::call_regs(0x004389a5, call);
    GP4_GLOBAL(uint32_t, 0x00629f40) = saved;
    GP4_GLOBAL(uint8_t, 0x00629f30) = 0;

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = call.eflags;
}
GP4_IMPL(0x00437d47, FUN_00437d47, "regs(esi:ptr[0x41c]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00437d47")
