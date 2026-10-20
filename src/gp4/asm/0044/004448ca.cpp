#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// `imul eax` then `shrd eax, edx, 6`: edx:eax = v * v (signed); returns the new eax and the high half in *high.
static inline uint32_t squareShr6(int32_t v, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(v) * static_cast<int64_t>(v);
    const uint32_t lo = static_cast<uint32_t>(product);
    const uint32_t hi = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    *high = hi;
    return (lo >> 6) | (hi << 26);
}

// Accumulates one 16-byte sample (three dwords at ebx + esi*16) into the running sums: the plain sums
// 0x0062a270 / 0x0062a288 / 0x0062a27c, the sums of squares >> 6 (64-bit square, shrd) 0x0062a28c /
// 0x0062a290 / 0x0062a294 and the sample count 0x0062a2b0. esi and ebx are preserved; eax / edx end as the
// last shrd result and the high half of the last square; CF is the carry of the last sum-of-squares add
// (the final inc leaves it alone), the other flags are those of the inc.
// FUNCTION: GP4 0x004448ca
void __cdecl FUN_004448ca(gp4::Regs* r) {
    const int32_t* sample = reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(r->ebx) + (r->esi << 4));
    uint32_t high = 0;
    uint32_t squared = 0;

    GP4_GLOBAL(uint32_t, 0x0062a270) += static_cast<uint32_t>(sample[0]);
    squared = squareShr6(sample[0], &high);
    GP4_GLOBAL(uint32_t, 0x0062a28c) += squared;

    GP4_GLOBAL(uint32_t, 0x0062a288) += static_cast<uint32_t>(sample[1]);
    squared = squareShr6(sample[1], &high);
    GP4_GLOBAL(uint32_t, 0x0062a290) += squared;

    GP4_GLOBAL(uint32_t, 0x0062a27c) += static_cast<uint32_t>(sample[2]);
    squared = squareShr6(sample[2], &high);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062a294);
    const uint32_t sum = before + squared;
    GP4_GLOBAL(uint32_t, 0x0062a294) = sum;
    const uint32_t carry = sum < before ? 1u : 0u;

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062a2b0);
    const uint32_t incremented = count + 1u;
    GP4_GLOBAL(uint32_t, 0x0062a2b0) = incremented;
    const uint32_t flags = carry
        | parityFlag(incremented)
        | (((count ^ 1u ^ incremented) & 0x10u) != 0u ? 0x10u : 0u)
        | (incremented == 0u ? 0x40u : 0u)
        | ((incremented & 0x80000000u) != 0u ? 0x80u : 0u)
        | (count == 0x7fffffffu ? 0x800u : 0u);

    r->eax = squared;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004448ca, FUN_004448ca, "regs(ebx:ptr[0x108]:bytes, esi:u32[0..15]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
