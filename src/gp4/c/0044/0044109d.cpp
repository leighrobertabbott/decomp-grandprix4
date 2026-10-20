#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) << 4);
}

// Resets car record 0 (0x0066fef0) at the start of a session: the divisor ebp = [0x60138c] (plus the addend
// [0x62eca8] when the byte [0x622426] is 0xff, which also sets bit 0 of +0x8d) stores its low word at +0xb2 and
// the quotient of ([0x60138c] << 14) / ebp at +0xd6; then the setup 0x00410a65 (with the byte [0x601f20] raised
// to 0xff around it) and 0x0044138b run for the record, and a set of words is reset or set to 0x4000 / 0x1000.
// With [0x622426] == 0xff the word +0x10e takes [0x62eca4] and eax holds that value. eax / ebx / edx are those
// 0x0044138b leaves, ebp is the divisor, esi the record; the flags are those of the final byte compare.
// FUNCTION: GP4 0x0044109d
void __cdecl FUN_0044109d(gp4::Regs* r) {
    uint8_t* const car = GP4_ARRAY(uint8_t, 0x0066fef0);

    GP4_FIELD(uint8_t, car, 0x7c) = 1;
    const int32_t dividend = GP4_GLOBAL(int32_t, 0x0060138c);
    int32_t divisor = dividend;
    if (GP4_GLOBAL(uint8_t, 0x00622426) == 0xffu) {
        GP4_FIELD(uint8_t, car, 0x8d) |= 1;
        divisor = (int32_t)((uint32_t)dividend + (uint32_t)GP4_GLOBAL(int32_t, 0x0062eca8));
    }
    GP4_FIELD(uint16_t, car, 0xb2) = (uint16_t)divisor;

    int32_t rem;
    const int32_t quotient = gp4::idiv64((int64_t)GP4_GLOBAL(int32_t, 0x0060138c) << 14, divisor, &rem);
    GP4_FIELD(uint16_t, car, 0xd6) = (uint16_t)quotient;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (uint32_t)quotient;
    c.edx = (uint32_t)rem;
    c.ebp = (uint32_t)divisor;
    c.esi = 0x0066fef0u;

    GP4_GLOBAL(uint8_t, 0x00601f20) = 0xff;
    gp4::call_regs(0x00410a65, c);
    GP4_GLOBAL(uint8_t, 0x00601f20) = 0;
    gp4::call_regs(0x0044138b, c);

    GP4_FIELD(uint16_t, car, 0x108) = 0;
    GP4_FIELD(uint16_t, car, 0x1ac) = 0;
    GP4_FIELD(uint16_t, car, 0x10a) = 0x4000;
    GP4_FIELD(uint8_t, car, 0x295) = 0;
    GP4_FIELD(uint16_t, car, 0xf6) = 0x4000;
    GP4_FIELD(uint16_t, car, 0x10e) = 0x4000;
    GP4_FIELD(uint16_t, car, 0xee) = 0x4000;
    GP4_FIELD(uint16_t, car, 0xa0) = 0x4000;
    GP4_FIELD(uint16_t, car, 0x24c) = 0x1000;
    GP4_FIELD(uint16_t, car, 0x24e) = 0x1000;

    const uint32_t flags = cmpFlags8(GP4_GLOBAL(uint8_t, 0x00622426), 0xffu);
    if (GP4_GLOBAL(uint8_t, 0x00622426) == 0xffu) {
        c.eax = GP4_GLOBAL(uint32_t, 0x0062eca4);
        GP4_FIELD(uint16_t, car, 0x10e) = (uint16_t)c.eax;
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->edx = c.edx;
    r->ebp = (uint32_t)divisor;
    r->esi = 0x0066fef0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044109d, FUN_0044109d, "regs() -> (eax, ebx, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+state_0040fbf3+wheel_00410a1c+state_0043c1ba+state_0043e0bc")
