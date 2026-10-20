#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// imul + shrd eax, edx, shift : the low dword of the 64-bit signed product shifted right
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// FUNCTION: GP4 0x00415b00
// Steering/pedal style rate limiter on a car record: d = word +0x264 - word +0x62 (target minus
// current) is limited to +-(rate * 0x603854 >> 16), where the rate is picked by the signs and the
// state bits 0x10/0x20 of byte +0x232 (bit 0x10 / 0x20 are cleared when d reaches the limits
// 0x603abc / 0x603ab8). Returns eax = limited d + word +0x62 and edx = that sum scaled by
// 0x603aa4 (Q14, at most 0x4000, divided by 8 while bit 0x10 is set). ebp is saved and restored;
// the flags are those of the last test of bit 0x10 or of the sar by 3.
void __cdecl FUN_00415b00(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t rate = GP4_GLOBAL(int32_t, 0x00603a6c);
    if ((GP4_FIELD(uint8_t, car, 0x232) & 0x20u) != 0u) {
        rate = GP4_GLOBAL(int32_t, 0x00603a68);
    }

    const uint32_t current = GP4_FIELD(uint16_t, car, 0x62);
    const int32_t diff = static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0x264)) - static_cast<int32_t>(current);
    if (diff < 0) {
        rate = GP4_GLOBAL(int32_t, 0x00603a70);
        if ((GP4_FIELD(uint8_t, car, 0x232) & 0x10u) != 0u) {
            rate = GP4_GLOBAL(int32_t, 0x00603a74);
        }
    }

    if (diff >= GP4_GLOBAL(int32_t, 0x00603abc)) {
        GP4_FIELD(uint8_t, car, 0x232) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x232) & 0xefu);
    }
    if (diff <= GP4_GLOBAL(int32_t, 0x00603ab8)) {
        GP4_FIELD(uint8_t, car, 0x232) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x232) & 0xdfu);
    }

    const uint32_t limit = mulShift(rate, GP4_GLOBAL(int32_t, 0x00603854), 16);
    uint32_t step = static_cast<uint32_t>(diff);
    if (diff < 0) {
        uint32_t magnitude = 0u - step;
        if (!(magnitude < limit)) magnitude = limit;
        step = 0u - magnitude;
    } else if (!(step < limit)) {
        step = limit;
    }

    const uint32_t sum = step + current;
    int32_t scaled = static_cast<int32_t>(mulShift(static_cast<int32_t>(sum), GP4_GLOBAL(int32_t, 0x00603aa4), 14));
    if (scaled > 0x4000) scaled = 0x4000;

    uint32_t flags = 0x44u;     // test byte,0x10 with the bit clear: ZF and PF
    if ((GP4_FIELD(uint8_t, car, 0x232) & 0x10u) != 0u) {
        const int32_t shifted = scaled >> 3;
        flags = (((static_cast<uint32_t>(scaled) >> 2) & 1u) != 0u ? 1u : 0u)
            | parityFlag(static_cast<uint32_t>(shifted))
            | (shifted == 0 ? 0x40u : 0u)
            | ((static_cast<uint32_t>(shifted) >> 24) & 0x80u);
        scaled = shifted;
    }

    r->eax = sum;
    r->edx = static_cast<uint32_t>(scaled);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415b00, FUN_00415b00, "regs(esi:ptr[0x26c]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
