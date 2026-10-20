#include <gp4/gp4.h>
#include <gp4/x87.h>

// One interpolation step: (hi - lo) is stored to the scratch dword 0x006f2bf4, reloaded with
// FILD, multiplied by the qword factor (80-bit product) and rounded with FISTP back into the
// same dword. The caller adds lo to the result.
static inline int32_t scaledDelta(int32_t lo, int32_t hi, double factor) {
    GP4_GLOBAL(int32_t, 0x006f2bf4) = hi - lo;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = gp4::x87::fistp32(
        gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * gp4::x87::ext(factor));
    return GP4_GLOBAL(int32_t, 0x006f2bf4);
}

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00407ed5
void __cdecl FUN_00407ed5(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x0062799c) = scaledDelta(GP4_GLOBAL(int32_t, 0x006279ac), GP4_GLOBAL(int32_t, 0x006279b0), GP4_GLOBAL(double, 0x00627958)) + GP4_GLOBAL(int32_t, 0x006279ac);
    GP4_GLOBAL(int32_t, 0x006279a0) = scaledDelta(GP4_GLOBAL(int32_t, 0x006279cc), GP4_GLOBAL(int32_t, 0x006279d0), GP4_GLOBAL(double, 0x00627958)) + GP4_GLOBAL(int32_t, 0x006279cc);
    GP4_GLOBAL(int32_t, 0x006279a4) = scaledDelta(GP4_GLOBAL(int32_t, 0x006279b8), GP4_GLOBAL(int32_t, 0x006279b4), GP4_GLOBAL(double, 0x00627958)) + GP4_GLOBAL(int32_t, 0x006279b8);
    GP4_GLOBAL(int32_t, 0x006279a8) = scaledDelta(GP4_GLOBAL(int32_t, 0x006279d8), GP4_GLOBAL(int32_t, 0x006279d4), GP4_GLOBAL(double, 0x00627958)) + GP4_GLOBAL(int32_t, 0x006279d8);
    const int32_t first = scaledDelta(GP4_GLOBAL(int32_t, 0x0062799c), GP4_GLOBAL(int32_t, 0x006279a4), GP4_GLOBAL(double, 0x00627950)) + GP4_GLOBAL(int32_t, 0x0062799c);

    // second row: edx = rounded delta + base; the flags of that final add stay behind
    const uint32_t delta = static_cast<uint32_t>(scaledDelta(GP4_GLOBAL(int32_t, 0x006279a0), GP4_GLOBAL(int32_t, 0x006279a8), GP4_GLOBAL(double, 0x00627950)));
    const uint32_t base = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279a0));
    const uint32_t second = delta + base;
    const uint32_t flags = (second < delta ? 0x1u : 0u)
        | parityFlag(second)
        | (((delta ^ base ^ second) & 0x10u) != 0u ? 0x10u : 0u)
        | (second == 0u ? 0x40u : 0u)
        | ((second & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((delta ^ second) & (base ^ second)) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = static_cast<uint32_t>(first);
    r->edx = second;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00407ed5, FUN_00407ed5, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
