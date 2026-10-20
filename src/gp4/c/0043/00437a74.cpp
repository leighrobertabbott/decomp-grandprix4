#include <gp4/gp4.h>

// Q14 table word at byte offset (|angle| >> 2) & 0xfffe (16-bit neg, shr, and), angle a 16-bit word.
static inline int16_t tableAt(uint16_t angle) {
    if ((angle & 0x8000u) != 0u) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t index = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), index);
}

// low 32 bits of the signed 64-bit product shifted right by 14 (imul + shrd eax, edx, 0xe).
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 14);
}

// 16-bit "imul word": the signed 32-bit product dx:ax.
static inline uint32_t mul16(uint16_t a, uint16_t b) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(a)) * static_cast<int32_t>(static_cast<int16_t>(b)));
}

static inline uint32_t absolute(int32_t value) {
    return value < 0 ? 0u - static_cast<uint32_t>(value) : static_cast<uint32_t>(value);
}

// Rotates the pair (x, y) = (words 0x934df0, 0x934df4) by (cos, sin) = (0x934de8, 0x934dec) into the
// dwords 0x934df0 = x*cos - y*sin and 0x934df4 = y*cos + x*sin, then keeps bits 14..29 of each in
// the low words (shl/rcl twice; the high words keep the products' high halves).
static inline void rotate() {
    const uint16_t x = GP4_GLOBAL(uint16_t, 0x00934df0);
    const uint16_t y = GP4_GLOBAL(uint16_t, 0x00934df4);
    const uint16_t c = GP4_GLOBAL(uint16_t, 0x00934de8);
    const uint16_t s = GP4_GLOBAL(uint16_t, 0x00934dec);
    const uint32_t first = mul16(x, c) - mul16(y, s);
    const uint32_t second = mul16(y, c) + mul16(x, s);
    GP4_GLOBAL(uint16_t, 0x00934df2) = static_cast<uint16_t>(first >> 16);
    GP4_GLOBAL(uint16_t, 0x00934df6) = static_cast<uint16_t>(second >> 16);
    GP4_GLOBAL(uint16_t, 0x00934df4) = static_cast<uint16_t>(second >> 14);
    GP4_GLOBAL(uint16_t, 0x00934df0) = static_cast<uint16_t>(first >> 14);
}

// FUNCTION: GP4 0x00437a74
// pushal ... popal routine on the car record at esi. The vector (+0xa6, +0xaa) is rotated by the angle
// word 0x629dd2 (Q14 cosine/sine from the table 0x7c5c5c); the first component, floored at word
// 0x629f2a (capped at -0x629f2a when word +0x90 is negative), goes to 0x629dce, the second to
// 0x629dd0. The first becomes -0x629dce * [0x629e70] >> 14, the second 0x629dd0 * (0x4000 -
// cos(0x4000 - angle(|0x629dce|, |0x629dd0|)) * [0x629e84] >> 14) >> 14 (angle from 0x401315); the
// pair is rotated back (negated sine) into +0xa6/+0xaa. Then 0x437d47 is called; when esi is the
// car in 0x679d30 and +0x7a bit 3 is clear, 0x852dbc = 0x66d814 + 500; then 0x418c68 is called.
// Every register is restored; the flags are those 0x418c68 leaves.
void __cdecl FUN_00437a74(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    const uint16_t heading = GP4_GLOBAL(uint16_t, 0x00629dd2);
    GP4_GLOBAL(int16_t, 0x00934de8) = tableAt(heading);
    GP4_GLOBAL(int16_t, 0x00934dec) = tableAt(static_cast<uint16_t>(0x4000u - heading));

    GP4_GLOBAL(uint16_t, 0x00934df0) = GP4_FIELD(uint16_t, car, 0xa6);
    GP4_GLOBAL(uint16_t, 0x00934df4) = GP4_FIELD(uint16_t, car, 0xaa);
    rotate();

    int16_t lateral = GP4_GLOBAL(int16_t, 0x00934df0);
    int16_t limit = GP4_GLOBAL(int16_t, 0x00629f2a);
    if (GP4_FIELD(int16_t, car, 0x90) >= 0) {
        if (lateral < limit) {
            lateral = limit;
        }
    } else {
        limit = static_cast<int16_t>(0u - static_cast<uint16_t>(limit));
        if (lateral > limit) {
            lateral = limit;
        }
    }
    GP4_GLOBAL(int16_t, 0x00629dce) = lateral;
    GP4_GLOBAL(uint16_t, 0x00934df0) = static_cast<uint16_t>(
        mulShr14(-static_cast<int32_t>(lateral), GP4_GLOBAL(int32_t, 0x00629e70)));

    const int16_t forward = GP4_GLOBAL(int16_t, 0x00934df4);
    GP4_GLOBAL(int16_t, 0x00629dd0) = forward;

    c.edx = absolute(forward);
    c.eax = absolute(GP4_GLOBAL(int16_t, 0x00629dce));
    gp4::call_regs(0x00401315, c);

    const int32_t cosine = tableAt(static_cast<uint16_t>(0x4000u - c.eax));
    const uint32_t factor = 0x4000u - mulShr14(cosine, GP4_GLOBAL(int32_t, 0x00629e84));
    GP4_GLOBAL(uint16_t, 0x00934df4) = static_cast<uint16_t>(
        mulShr14(static_cast<int32_t>(factor), GP4_GLOBAL(int16_t, 0x00629dd0)));

    GP4_GLOBAL(uint16_t, 0x00934dec) = static_cast<uint16_t>(0u - GP4_GLOBAL(uint16_t, 0x00934dec));
    rotate();

    GP4_FIELD(uint16_t, car, 0xa6) = GP4_GLOBAL(uint16_t, 0x00934df0);
    GP4_FIELD(uint16_t, car, 0xaa) = GP4_GLOBAL(uint16_t, 0x00934df4);

    gp4::call_regs(0x00437d47, c);

    if (r->esi == GP4_GLOBAL(uint32_t, 0x00679d30) && (car[0x7a] & 8u) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00852dbc) = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_GLOBAL(uint32_t, 0x00852dbc) += 0x1f4u;
    }

    gp4::call_regs(0x00418c68, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00437a74, FUN_00437a74, "regs(esi:ptr[0x41c]:layout=car_004129fa+rec_00418c68) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00437d47+state_00418c68+state_0043247d")
