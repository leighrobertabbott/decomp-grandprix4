#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// One clamped output: ((sample * multiplier / 0xf0) * rate) >> 14 (64-bit imul, SHRD 14), capped at 0xd70
// (signed). `flags` receives the flags of the CMP against 0xd70.
static inline int32_t clampedRate(uint8_t sample, uint32_t multiplier, int32_t rate, uint32_t* flags) {
    const uint64_t product = static_cast<uint64_t>(sample) * multiplier;
    uint32_t remainder;
    const uint32_t quotient = gp4::div64(product, 0xf0u, &remainder);
    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(quotient)) * rate;
    const uint32_t low = static_cast<uint32_t>(scaled);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 32);
    const uint32_t value = (low >> 14) | (high << 18);
    *flags = cmpFlags32(value, 0xd70u);
    return static_cast<int32_t>(value) > 0xd70 ? 0xd70 : static_cast<int32_t>(value);
}

// Four per-axis ratios written to 0x007a9114/18/1c/20 for the record in esi: the row is
// (byte +0x7c & 0x3f) - 1 clamped to 0x15, the column sel = byte [0x66d936 + row], the multiplier the dword
// [0x66ec60 + (row*0x2c + sel)*4] >> 8 (arithmetic), each ratio from the byte tables 0x66dd2a / 0x66e0f2 /
// 0x66e4ba / 0x66e882 at [row*0x2c + sel] times that multiplier divided by 0xf0, scaled by [0x601d54]
// (rate, >> 14) and capped at 0xd70. pushal/popal keep every register; the flags are those of the last
// cmp against 0xd70.
// FUNCTION: GP4 0x00441606
void __cdecl FUN_00441606(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t row = static_cast<int32_t>(rec[0x7c] & 0x3fu) - 1;
    if (row > 0x15) {
        row = 0x15;
    }
    const int32_t sel = GP4_ARRAY(uint8_t, 0x0066d936)[row];
    const int32_t cell = row * 0x2c + sel;
    const uint32_t multiplier = static_cast<uint32_t>(static_cast<int32_t>(GP4_ARRAY(uint32_t, 0x0066ec60)[cell]) >> 8);
    const int32_t rate = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00601d54));

    uint32_t flags = 0u;
    GP4_GLOBAL(int32_t, 0x007a9114) = clampedRate(GP4_ARRAY(uint8_t, 0x0066dd2a)[cell], multiplier, rate, &flags);
    GP4_GLOBAL(int32_t, 0x007a9118) = clampedRate(GP4_ARRAY(uint8_t, 0x0066e0f2)[cell], multiplier, rate, &flags);
    GP4_GLOBAL(int32_t, 0x007a911c) = clampedRate(GP4_ARRAY(uint8_t, 0x0066e4ba)[cell], multiplier, rate, &flags);
    GP4_GLOBAL(int32_t, 0x007a9120) = clampedRate(GP4_ARRAY(uint8_t, 0x0066e882)[cell], multiplier, rate, &flags);

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441606, FUN_00441606, "regs(esi:ptr[0x84]:layout=car_00441606) -> (cf, pf, af, zf, sf, of); globals=layout=counter_0040f0dd+tabs_004413b7+tabs_00441606")
