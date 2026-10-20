#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (to - from) * t >> 14 as 64-bit imul + shrd eax, edx, 14 (low 32 bits kept)
static inline uint32_t scaledDifference(uint32_t to, uint32_t from, int32_t t) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(to - from)) * t;
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0041e7f3
// pushal ... popal routine: builds three 4-dword blocks at 0x006279fc, 0x00627a0c and
// 0x00627a1c from the ring records (edi = [0x00866ecc] - 0x2c0, esi = 0x008943a0 records) and
// the 16-bit fraction at edi+0x290. Only the flags of the last "add" escape.
void __cdecl FUN_0041e7f3(gp4::Regs* r) {
    uint8_t* esi = GP4_ARRAY(uint8_t, 0x008943a0);
    uint8_t* edi = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x2c0u);
    uint32_t* out = GP4_ARRAY(uint32_t, 0x006279fc);

    const uint32_t fraction = GP4_FIELD(uint16_t, edi, 0x290);
    const int32_t inverse = static_cast<int32_t>(0x4000u - fraction);

    out[0] = GP4_FIELD(uint32_t, edi, -0x6c);
    out[1] = GP4_FIELD(uint32_t, edi, 0xf4);
    out[2] = GP4_FIELD(uint32_t, edi, 0x254);
    out[3] = scaledDifference(GP4_FIELD(uint32_t, esi, 0x254), GP4_FIELD(uint32_t, esi, 0xf4), inverse)
        + GP4_FIELD(uint32_t, esi, 0xf4);

    out[4] = GP4_FIELD(uint32_t, edi, 0xf4);
    out[5] = GP4_FIELD(uint32_t, edi, 0x254);
    out[6] = scaledDifference(GP4_FIELD(uint32_t, esi, 0x254), GP4_FIELD(uint32_t, esi, 0xf4), inverse)
        + GP4_FIELD(uint32_t, esi, 0xf4);
    out[7] = scaledDifference(GP4_FIELD(uint32_t, esi, 0x3b4), GP4_FIELD(uint32_t, esi, 0x254), inverse)
        + GP4_FIELD(uint32_t, esi, 0x254);

    const uint32_t scaled = scaledDifference(GP4_FIELD(uint32_t, edi, 0x254), GP4_FIELD(uint32_t, edi, 0xf4),
                                             static_cast<int32_t>(fraction));
    const uint32_t base = GP4_FIELD(uint32_t, edi, 0xf4);
    out[8] = scaled + base;
    out[9] = GP4_FIELD(uint32_t, esi, 0xf4);
    out[10] = GP4_FIELD(uint32_t, esi, 0x254);
    out[11] = GP4_FIELD(uint32_t, esi, 0x3b4);

    // popal restores the registers; the flags are those of the last add (out[8])
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(scaled, base);
}
GP4_IMPL(0x0041e7f3, FUN_0041e7f3, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=records_008943a0+dvar_0041eacf")
