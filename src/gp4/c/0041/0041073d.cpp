#include <gp4/gp4.h>

// Only the fields this routine reads or writes (four per-wheel dwords at each offset).
struct Unknown_0041073d {
    uint8_t Unknown000[0x2cc];
    uint32_t Divisor2cc[4];
    uint8_t Unknown2dc[0x10];
    uint32_t Value2ec[4];
    uint32_t Result2fc[4];
    uint32_t Zero30c;
    uint8_t Unknown310[0xc];
    uint32_t Quotient31c[4];
    uint32_t Quotient32c[4];
    uint32_t Subtrahend33c[4];
    uint32_t Zero34c;
    uint8_t Unknown350[0x2c];
    uint32_t Addend37c[4];
};
static_assert(sizeof(Unknown_0041073d) == 0x38c, "layout");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041073d
// esi = wheel body. 0x0041062a refreshes the four loads at +0x2ec; then for wheel w = 0..3:
//   [+0x31c+4w] = (load << 10) / DAT_00601d9c[w]
//   [+0x32c+4w] = ((load << 6) / [+0x2cc+4w]) << 4
//   [+0x2fc+4w] = DAT_00601df4[w] - [+0x32c+4w] - [+0x31c+4w] - [+0x33c+4w] + [+0x37c+4w]
//                 + (+0x2c4 of the 0x300-byte record w at 0x006380d8)
// and +0x30c, +0x34c are cleared. The mean of wheels 2/3 minus the mean of wheels 0/1 is clamped to
// +-0x80000, shifted left 26 and divided by DAT_00638da8 + DAT_00638da4; the quotient is added to
// DAT_00638034. pushal/popal around everything: registers unchanged, flags of that final add.
void __cdecl FUN_0041073d(gp4::Regs* r) {
    Unknown_0041073d* body = reinterpret_cast<Unknown_0041073d*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs refresh = *r;
    refresh.st_in = 0;
    refresh.st_out = 0;
    gp4::call_regs(0x0041062a, refresh);

    uint8_t* wheels = GP4_ARRAY(uint8_t, 0x006380d8);
    for (uint32_t w = 0; w != 4; ++w) {
        const uint32_t load = body->Value2ec[w];
        uint32_t remainder;
        body->Quotient31c[w] = gp4::div64(static_cast<uint64_t>(load) << 10, GP4_ARRAY(uint32_t, 0x00601d9c)[w], &remainder);
        const uint32_t scaled = gp4::div64(static_cast<uint64_t>(body->Value2ec[w]) << 6, body->Divisor2cc[w], &remainder);
        body->Quotient32c[w] = scaled << 4;
        uint32_t sum = GP4_ARRAY(uint32_t, 0x00601df4)[w];
        sum -= body->Quotient32c[w];
        sum -= body->Quotient31c[w];
        sum -= body->Subtrahend33c[w];
        sum += body->Addend37c[w];
        sum += GP4_FIELD(uint32_t, wheels + w * 0x300, 0x2c4);
        body->Result2fc[w] = sum;
        body->Zero34c = 0;
        body->Zero30c = 0;
    }

    const int32_t rear = static_cast<int32_t>(body->Result2fc[2] + body->Result2fc[3]) >> 1;
    const int32_t front = static_cast<int32_t>(body->Result2fc[0] + body->Result2fc[1]) >> 1;
    int32_t diff = static_cast<int32_t>(static_cast<uint32_t>(rear) - static_cast<uint32_t>(front));
    if (diff < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(diff);
        if (magnitude >= 0x80000u) {
            magnitude = 0x80000u;
        }
        diff = static_cast<int32_t>(0u - magnitude);
    } else if (static_cast<uint32_t>(diff) >= 0x80000u) {
        diff = 0x80000;
    }

    const int32_t divisor = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00638da8) + GP4_GLOBAL(uint32_t, 0x00638da4));
    int32_t remainder;
    const uint32_t quotient = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(diff) << 26, divisor, &remainder));
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00638034);
    GP4_GLOBAL(uint32_t, 0x00638034) = before + quotient;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, quotient);
}
GP4_IMPL(0x0041073d, FUN_0041073d, "regs(esi:ptr[0x3a0]:layout=rec_004105c7+rec_004195e1+car_00402db6, edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00405489+state_004105c7+recs_006380d8+integ_004200a7+engine_00413606+ring_008943a0+bank_004279ab+state_0041062a+tune_004057cf")
