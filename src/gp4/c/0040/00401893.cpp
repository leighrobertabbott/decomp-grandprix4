#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// (a * b) >> 14 on the signed 64-bit product, low 32 bits kept (imul / shrd eax, edx, 14)
static inline uint32_t term(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00401893
// Q14 product of the vector (0x00623e24, 0x00623e28, 0x00623e2c) with the 3x3 block at 0x00623e3c..:
// 0x00623e30 / 34 / 38 = e24*m[k][0] + e28*m[k][1] + e2c*m[k][2] for the columns 3c/48/54, 40/4c/58
// and 44/50/5c; each product is shifted on its own.
// The original leaves the last product's high half in edx, the first two terms' sum in ebp,
// the result in eax and the flags of the final add.
void __cdecl FUN_00401893(gp4::Regs* r) {
    const int32_t v0 = GP4_GLOBAL(int32_t, 0x00623e24);
    const int32_t v1 = GP4_GLOBAL(int32_t, 0x00623e28);
    const int32_t v2 = GP4_GLOBAL(int32_t, 0x00623e2c);
    uint32_t high;

    uint32_t t0 = term(v0, GP4_GLOBAL(int32_t, 0x00623e3c), &high);
    uint32_t t1 = term(v1, GP4_GLOBAL(int32_t, 0x00623e48), &high);
    uint32_t partial = t0 + t1;
    uint32_t t2 = term(v2, GP4_GLOBAL(int32_t, 0x00623e54), &high);
    uint32_t sum = t2 + partial;
    GP4_GLOBAL(uint32_t, 0x00623e30) = sum;

    t0 = term(v0, GP4_GLOBAL(int32_t, 0x00623e40), &high);
    t1 = term(v1, GP4_GLOBAL(int32_t, 0x00623e4c), &high);
    partial = t0 + t1;
    t2 = term(v2, GP4_GLOBAL(int32_t, 0x00623e58), &high);
    sum = t2 + partial;
    GP4_GLOBAL(uint32_t, 0x00623e34) = sum;

    t0 = term(v0, GP4_GLOBAL(int32_t, 0x00623e44), &high);
    t1 = term(v1, GP4_GLOBAL(int32_t, 0x00623e50), &high);
    partial = t0 + t1;
    t2 = term(v2, GP4_GLOBAL(int32_t, 0x00623e5c), &high);
    sum = t2 + partial;
    GP4_GLOBAL(uint32_t, 0x00623e38) = sum;

    // flags of the final "add eax, ebp"
    const uint32_t flags = (sum < partial ? 0x01u : 0u)
        | parityFlag(sum)
        | (((t2 ^ partial ^ sum) & 0x10u) != 0u ? 0x10u : 0u)
        | (sum == 0u ? 0x40u : 0u)
        | ((sum & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((t2 ^ sum) & (partial ^ sum)) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = sum;
    r->edx = high;
    r->ebp = partial;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00401893, FUN_00401893, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
