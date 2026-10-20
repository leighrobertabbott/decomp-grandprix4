#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left - right` (cmp).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040cc58
// pushal ... popal wrapper (all registers preserved; flags are those of the last cmp executed).
// Splits the table byte 0x62eade[DAT_007ad894] << 8, divided by max(eax, 1) + 1, into six
// dwords at edi+4..0x18: +8/+0x10/+0x18 = (q * 3 >> 4) + 0x80 >> 8 and +4/+0xc/+0x14 =
// the same ramp q - (q*3 >> 5), then + q, + q (each (x + 0x80) >> 8, capped at 99).
void __cdecl FUN_0040cc58(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    int32_t divisor = static_cast<int32_t>(r->eax);
    if (divisor < 1) {
        divisor = 1;
    }
    divisor = static_cast<int32_t>(static_cast<uint32_t>(divisor) + 1u);

    const uint32_t row = static_cast<uint32_t>(GP4_ARRAY(const uint8_t, 0x0062eade)[GP4_GLOBAL(uint32_t, 0x007ad894)]) << 8;
    int32_t remainder;
    const uint32_t q = static_cast<uint32_t>(gp4::idiv64(static_cast<int64_t>(row), divisor, &remainder));

    uint32_t ramp = (q * 3u) >> 4;
    const uint32_t base = (ramp + 0x80u) >> 8;
    GP4_FIELD(uint32_t, out, 0x08) = base;
    GP4_FIELD(uint32_t, out, 0x10) = base;
    GP4_FIELD(uint32_t, out, 0x18) = base;

    ramp = (0u - (ramp >> 1)) + q;
    uint32_t step = (ramp + 0x80u) >> 8;
    GP4_FIELD(uint32_t, out, 0x04) = step > 0x63u ? 0x63u : step;

    ramp += q;
    step = (ramp + 0x80u) >> 8;
    GP4_FIELD(uint32_t, out, 0x0c) = step > 0x63u ? 0x63u : step;

    ramp += q;
    step = (ramp + 0x80u) >> 8;
    GP4_FIELD(uint32_t, out, 0x14) = step > 0x63u ? 0x63u : step;

    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(step, 0x63u);
}
GP4_IMPL(0x0040cc58, FUN_0040cc58, "regs(eax:i32, edi:ptr[0x20]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=scale_0062eade+trk_007ad894")
