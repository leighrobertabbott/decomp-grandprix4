#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit cmp left, right as seen after "clc": CF is cleared, the rest is the compare.
static inline uint32_t cmpFlags16NoCarry(uint16_t left, uint16_t right) {
    const uint32_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of a 32-bit xor/and/test result: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u);
}

// The word difference of the two bodies: [ebx+0xe2] - [esi+0xe2], or the result of 0x40dc8a when
// either body has bit 1 of byte +0x9c set. `live` is the register file at that point.
static uint16_t bodyDistance(const gp4::Regs& live, uint8_t* ebx, uint8_t* esi) {
    if ((GP4_FIELD(uint8_t, esi, 0x9c) & 2u) != 0u || (GP4_FIELD(uint8_t, ebx, 0x9c) & 2u) != 0u) {
        gp4::Regs call = live;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0040dc8a, call);
        return static_cast<uint16_t>(call.eax);
    }
    return static_cast<uint16_t>(GP4_FIELD(uint16_t, ebx, 0xe2) - GP4_FIELD(uint16_t, esi, 0xe2));
}

// FUNCTION: GP4 0x00425473
void __cdecl FUN_00425473(gp4::Regs* r) {
    uint8_t* ebx = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* esi = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    // Gate 1: the +0x68 words must differ by less than [0x603afc].
    const uint16_t gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, ebx, 0x68) - GP4_FIELD(uint16_t, esi, 0x68));
    const uint16_t gapLimit = GP4_GLOBAL(uint16_t, 0x00603afc);
    if (!(static_cast<int16_t>(gap) < static_cast<int16_t>(gapLimit))) {
        r->edx = (r->edx & 0xffff0000u) | gap;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags16NoCarry(gap, gapLimit);
        return;
    }

    // Gate 2: the distance of the bodies must reach [0x603b66].
    const uint16_t distance = bodyDistance(*r, ebx, esi);
    const uint16_t distanceLimit = GP4_GLOBAL(uint16_t, 0x00603b66);
    if (static_cast<int16_t>(distance) < static_cast<int16_t>(distanceLimit)) {
        r->edx = (r->edx & 0xffff0000u) | distance;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags16NoCarry(distance, distanceLimit);
        return;
    }

    // Gate 3: the sign of word +0xda must equal the sign of 0x100^2 - 0x9a^2 (ebx).
    const int32_t speedB = GP4_FIELD(int16_t, ebx, 0x9a);
    const int32_t squareB = speedB * speedB;
    const int32_t lateralB = GP4_FIELD(int16_t, ebx, 0x100);
    const int32_t difference = lateralB * lateralB - squareB;
    const int32_t slope = GP4_FIELD(int16_t, ebx, 0xda);
    const uint32_t gate = static_cast<uint32_t>(slope) ^ static_cast<uint32_t>(difference);
    if (static_cast<int32_t>(gate) < 0) {
        r->eax = static_cast<uint32_t>(difference);
        r->edx = gate;
        r->ebp = static_cast<uint32_t>(squareB);
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags32(gate);
        return;
    }

    // divisor = (slope << 17) / [0x603854] (1 when zero); quotient = difference / divisor >> 4,
    // clamped to +-0x4000.
    const int32_t rate = GP4_GLOBAL(int32_t, 0x00603854);
    int32_t remainder;
    int32_t divisor = gp4::idiv64(static_cast<int64_t>(slope) << 17, rate, &remainder);
    if (divisor == 0) {
        divisor = 1;
    }
    const int32_t quotient = difference / divisor;
    const int32_t remainder2 = difference % divisor;
    const int32_t shifted = quotient >> 4;
    uint32_t magnitude;
    int32_t clamped;
    if (shifted < 0) {
        magnitude = 0u - static_cast<uint32_t>(shifted);
        if (!(magnitude < 0x4000u)) {
            magnitude = 0x4000u;
        }
        clamped = static_cast<int32_t>(0u - magnitude);
    } else {
        magnitude = static_cast<uint32_t>(shifted);
        if (!(magnitude < 0x4000u)) {
            magnitude = 0x4000u;
        }
        clamped = static_cast<int32_t>(magnitude);
    }

    // Second distance (live registers: eax = clamped, edx = remainder, ebp = divisor).
    gp4::Regs live = *r;
    live.eax = static_cast<uint32_t>(clamped);
    live.edx = static_cast<uint32_t>(remainder2);
    live.ebp = static_cast<uint32_t>(divisor);
    const int32_t distance2 = static_cast<int16_t>(bodyDistance(live, ebx, esi));
    uint32_t scale = (static_cast<uint32_t>(clamped) + static_cast<uint32_t>(distance2)) << 1;
    if (scale == 0u) {
        scale = 1u;
    }

    // (0x100^2 - 0x9a^2 (esi)) / scale * [0x603854] >> 20, limited to +-|word 0x623e02|.
    const int32_t speedS = GP4_FIELD(int16_t, esi, 0x9a);
    const int32_t squareS = speedS * speedS;
    const int32_t numerator = lateralB * lateralB - squareS;
    const int32_t ratio = numerator / static_cast<int32_t>(scale);
    const int64_t product = static_cast<int64_t>(ratio) * static_cast<int64_t>(rate);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 20);
    int32_t bound = GP4_GLOBAL(int16_t, 0x00623e02);
    if (bound < 0) {
        bound = -bound;
    }
    uint32_t limited;
    if (static_cast<int32_t>(scaled) < 0) {
        limited = 0u - scaled;
        if (!(limited < static_cast<uint32_t>(bound))) {
            limited = static_cast<uint32_t>(bound);
        }
        limited = 0u - limited;
    } else {
        limited = scaled;
        if (!(limited < static_cast<uint32_t>(bound))) {
            limited = static_cast<uint32_t>(bound);
        }
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = limited;
    call.ecx = scale;
    call.edx = static_cast<uint32_t>(bound);
    call.ebp = static_cast<uint32_t>(squareS);
    gp4::call_regs(0x00426100, call);

    GP4_FIELD(uint16_t, esi, 0x21e) = static_cast<uint16_t>(call.eax);
    r->eax = call.eax;
    r->ecx = scale;
    r->edx = call.edx;
    r->ebp = static_cast<uint32_t>(squareS);
    // stc: the carry flag is set, the other flags are those of 0x426100
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d4u) | 1u;
}
GP4_IMPL(0x00425473, FUN_00425473, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, ebx:ptr[0x226]:layout=car_00425473, esi:ptr[0x226]:layout=rec_00426091+car_00425473) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00426091+cache_00426091+state_00425473")
