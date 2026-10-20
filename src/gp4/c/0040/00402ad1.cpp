#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The two rotated components, computed on the x87 stack at extended precision and stored with
// FISTP: 0x00622b4c = x*t40 - z*t38 and 0x00622b50 = z*t98 + x*t88 + y*t28, where x / y / z are
// the dwords 0x00622b40 / 44 / 48 and t is the table of doubles at 0x00622e98.
static __declspec(noinline) void rotateOffsets(uint8_t* table) {
    using namespace gp4::x87;
    const Ext x = ext_i32(GP4_GLOBAL(int32_t, 0x00622b40));
    const Ext xScaled = x * ext(GP4_FIELD(double, table, 0x40));
    const Ext z = ext_i32(GP4_GLOBAL(int32_t, 0x00622b48));
    const Ext zScaled = z * ext(GP4_FIELD(double, table, 0x38));
    GP4_GLOBAL(int32_t, 0x00622b4c) = fistp32(fchs(zScaled) + xScaled);
    const Ext zCross = z * ext(GP4_FIELD(double, table, 0x98));
    const Ext xCross = ext(GP4_FIELD(double, table, 0x88)) * x;
    const Ext y = ext_i32(GP4_GLOBAL(int32_t, 0x00622b44));
    const Ext yScaled = y * ext(GP4_FIELD(double, table, 0x28));
    GP4_GLOBAL(int32_t, 0x00622b50) = fistp32((zCross + xCross) + yScaled);
}

// FUNCTION: GP4 0x00402ad1
// When byte 0x0062599c is 0xff this is a tail wrapper around 0x00402b4e. Otherwise it rotates the
// offsets 0x00622b40/44/48 with the matrix at 0x00622e98 (rotateOffsets) and adds the results to
// the angles: 0x00638034 += 0x00622b4c, 0x00638038 += 0x00622b50, 0x00638030 -= 0x00622b44. On exit
// eax is -0x00622b44, edx the matrix address and the flags those of the last add.
void __cdecl FUN_00402ad1(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0062599c) == 0xffu) {
        gp4::call_regs(0x00402b4e, *r);
        return;
    }
    rotateOffsets(GP4_ARRAY(uint8_t, 0x00622e98));

    GP4_GLOBAL(uint32_t, 0x00638034) += GP4_GLOBAL(uint32_t, 0x00622b4c);
    GP4_GLOBAL(uint32_t, 0x00638038) += GP4_GLOBAL(uint32_t, 0x00622b50);
    const uint32_t negated = 0u - GP4_GLOBAL(uint32_t, 0x00622b44);
    const uint32_t flags = addFlags32(GP4_GLOBAL(uint32_t, 0x00638030), negated);
    GP4_GLOBAL(uint32_t, 0x00638030) += negated;

    r->eax = negated;
    r->edx = 0x00622e98u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00402ad1, FUN_00402ad1, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+trig_00622e98")
