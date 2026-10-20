#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
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

// Records reached through ebx = 0x006380d8: walk the pointer instead of using absolute addresses
// for each far field.
static __declspec(noinline) uint32_t negatedPairSum(const uint8_t* ebx) {
    const uint32_t first = GP4_FIELD(uint32_t, const_cast<uint8_t*>(ebx), 0x634);
    const uint32_t second = GP4_FIELD(uint32_t, const_cast<uint8_t*>(ebx), 0x934);
    return 0u - (first + second);
}

static __declspec(noinline) uint32_t offsetField(const uint8_t* ebx, uint32_t offset) {
    return *reinterpret_cast<const uint32_t*>(ebx + offset);
}

// DAT_006035e4 = (-(f634 + f934) * DAT_006013b0 >> 14) + f874 + fb74 over the block at 0x006380d8.
// ebx is left at the block, edx holds the high half of the product; the flags are those of the
// last add.
// FUNCTION: GP4 0x0041a236
void __cdecl FUN_0041a236(gp4::Regs* r) {
    const uint8_t* ebx = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(0x006380d8u));

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(negatedPairSum(ebx)))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006013b0));
    const uint32_t partial = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + offsetField(ebx, 0x874);
    const uint32_t last = offsetField(ebx, 0xb74);
    const uint32_t result = partial + last;
    GP4_GLOBAL(uint32_t, 0x006035e4) = result;

    r->ebx = 0x006380d8u;
    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(partial, last);
}
GP4_IMPL(0x0041a236, FUN_0041a236, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sums_0041a236")
