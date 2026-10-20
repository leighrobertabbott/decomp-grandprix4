#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The difference goes through the scratch dword 0x006f2bf4 (fild / fmul / fistp):
// scratch = round((from - to) * scale), the result of the fistp stays in the scratch dword.
static inline uint32_t scaledDifference(int32_t from, int32_t to) {
    using namespace gp4::x87;
    int32_t& scratch = GP4_GLOBAL(int32_t, 0x006f2bf4);
    scratch = static_cast<int32_t>(static_cast<uint32_t>(from) - static_cast<uint32_t>(to));
    scratch = fistp32(ext_i32(scratch) * ext(GP4_GLOBAL(double, 0x00627958)));
    return static_cast<uint32_t>(scratch);
}

// FUNCTION: GP4 0x00407944
// Four interpolations by the factor at 0x00627958: dest = round((a - b) * scale) + b.
// Leaves eax = the last dest and the flags of the final add.
void __cdecl FUN_00407944(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062799c) =
        scaledDifference(GP4_GLOBAL(int32_t, 0x006279b0), GP4_GLOBAL(int32_t, 0x006279ac)) + GP4_GLOBAL(uint32_t, 0x006279ac);
    GP4_GLOBAL(uint32_t, 0x006279a0) =
        scaledDifference(GP4_GLOBAL(int32_t, 0x006279d0), GP4_GLOBAL(int32_t, 0x006279cc)) + GP4_GLOBAL(uint32_t, 0x006279cc);
    GP4_GLOBAL(uint32_t, 0x006279a4) =
        scaledDifference(GP4_GLOBAL(int32_t, 0x006279b4), GP4_GLOBAL(int32_t, 0x006279b8)) + GP4_GLOBAL(uint32_t, 0x006279b8);

    const uint32_t scaled = scaledDifference(GP4_GLOBAL(int32_t, 0x006279d4), GP4_GLOBAL(int32_t, 0x006279d8));
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x006279d8);
    const uint32_t result = scaled + base;
    GP4_GLOBAL(uint32_t, 0x006279a8) = result;

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(scaled, base);
}
GP4_IMPL(0x00407944, FUN_00407944, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
