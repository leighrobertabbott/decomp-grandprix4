#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// FUNCTION: GP4 0x00421f53
void __cdecl FUN_00421f53(gp4::Regs* r) {
    const int32_t* src = reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(r->ebx));

    // |DAT_00623e1c| clamped to DAT_00608038 (signed compare)
    int32_t step = GP4_GLOBAL(int32_t, 0x00623e1c);
    if (step < 0) step = static_cast<int32_t>(0u - static_cast<uint32_t>(step));
    if (step > GP4_GLOBAL(int32_t, 0x00608038)) step = GP4_GLOBAL(int32_t, 0x00608038);

    // 64-bit sum at 0x006080fc: add / adc 0
    uint32_t oldLo = GP4_GLOBAL(uint32_t, 0x006080fc);
    uint32_t newLo = oldLo + static_cast<uint32_t>(step);
    GP4_GLOBAL(uint32_t, 0x006080fc) = newLo;
    GP4_GLOBAL(uint32_t, 0x00608100) += (newLo < oldLo) ? 1u : 0u;
    uint32_t carry = 0;

    uint32_t lo = 0, hi = 0, res = 0, prod_lo = 0, prod_hi = 0;
    static const uint32_t cur[3][2] = {
        {0x006097b4, 0x00608114}, {0x006097b8, 0x0060811c}, {0x006097bc, 0x00608124}};
    for (int k = 0; k < 3; ++k) {
        int32_t diff = static_cast<int32_t>(static_cast<uint32_t>(src[k]) - GP4_GLOBAL(uint32_t, cur[k][0]));
        int64_t prod = static_cast<int64_t>(diff) * static_cast<int64_t>(step);
        prod_lo = static_cast<uint32_t>(prod);
        prod_hi = static_cast<uint32_t>(static_cast<uint64_t>(prod) >> 32);
        uint32_t accLo = GP4_GLOBAL(uint32_t, cur[k][1]);
        uint32_t accHi = GP4_GLOBAL(uint32_t, cur[k][1] + 4);
        uint32_t newAccLo = accLo + prod_lo;
        GP4_GLOBAL(uint32_t, cur[k][1]) = newAccLo;
        carry = (newAccLo < accLo) ? 1u : 0u;
        lo = accLo;
        hi = accHi;
        res = accHi + prod_hi + carry;
        GP4_GLOBAL(uint32_t, cur[k][1] + 4) = res;
    }

    // flags of the final adc [0x00608128], edx
    uint64_t wide = static_cast<uint64_t>(hi) + prod_hi + carry;
    uint32_t flags = parityFlag(res);
    if (wide >> 32) flags |= 0x01u;
    if (((hi ^ prod_hi ^ res) >> 4) & 1u) flags |= 0x10u;
    if (res == 0u) flags |= 0x40u;
    if (res >> 31) flags |= 0x80u;
    if (((hi ^ res) & (prod_hi ^ res)) >> 31) flags |= 0x800u;
    (void)lo;

    r->eax = prod_lo;
    r->edx = prod_hi;
    r->ebp = static_cast<uint32_t>(step);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00421f53, FUN_00421f53,
         "regs(ebx:ptr[0x10]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
