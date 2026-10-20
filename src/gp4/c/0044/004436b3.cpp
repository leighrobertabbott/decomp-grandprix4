#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Gives each of the 22 car records at 0x66fef0 (stride 0x41c) a random start delay: +0xf0 =
// [0x66d814] + (((random byte << 6) * 2000) >> 14), and clears the byte +0x237. All registers are
// preserved (pushal/popal); the flags are those of the final `add esi, 0x41c`.
// FUNCTION: GP4 0x004436b3
void __cdecl FUN_004436b3(gp4::Regs* r) {
    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t flags = 0;

    for (uint32_t count = 22; count != 0; --count) {
        gp4::Regs rng = *r;
        gp4::call_regs(0x00401462, rng);

        const int32_t draw = static_cast<int32_t>((rng.eax & 0xffu) << 6);
        const int64_t product = static_cast<int64_t>(draw) * 0x7d0;
        const uint32_t offset = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

        GP4_FIELD(uint32_t, car, 0xf0) = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, car, 0xf0) += offset;
        GP4_FIELD(uint8_t, car, 0x237) = 0;

        flags = additionFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)), 0x41cu);
        car += 0x41c;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004436b3, FUN_004436b3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rng_0066d8c0")
