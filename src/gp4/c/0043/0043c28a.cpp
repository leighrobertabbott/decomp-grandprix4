#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t res) {
    return (res < a ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ res) & (b ^ res)) >> 31) << 11);
}

// FUNCTION: GP4 0x0043c28a
// Fixed-point (14 fractional bits) interpolation between two entries of two
// int32 tables (0x006227c4 and 0x0063b844), indexed by DAT_006227e8 / DAT_006227ec.
// The blend factor is (DAT_0062d238 << 15) / (max(DAT_00622778, 300) * 1000) times
// DAT_006227a0 (>> 14), clamped to 0..0x4000. The result in eax is
// ((A[i]-A[j]) - (B[i]-B[j])) * factor >> 14, plus (B[i]-B[j]); the flags are those
// of that final add, except CF which is set (stc) when the sign is clear, else clear.
// ebp, ebx, ecx and edx are preserved.
void __cdecl FUN_0043c28a(gp4::Regs* r) {
    int32_t scale = GP4_GLOBAL(int32_t, 0x00622778);
    if (scale < 300) {
        scale = 300;
    }
    const int32_t divisor = (int32_t)((uint32_t)scale * 1000u);
    const uint32_t dividend = GP4_GLOBAL(uint32_t, 0x0062d238);
    int32_t rem;
    const int32_t q = gp4::idiv64((int64_t)((uint64_t)dividend << 15), divisor, &rem);
    const int64_t prod1 = (int64_t)q * (int64_t)GP4_GLOBAL(int32_t, 0x006227a0);
    int32_t factor = (int32_t)((uint64_t)prod1 >> 14);
    if (factor < 0) {
        factor = 0;
    }
    if (factor > 0x4000) {
        factor = 0x4000;
    }
    const uint32_t i = GP4_GLOBAL(uint32_t, 0x006227e8);
    const uint32_t j = GP4_GLOBAL(uint32_t, 0x006227ec);
    const uint32_t* tabA = GP4_ARRAY(uint32_t, 0x006227c4);
    const uint32_t* tabB = GP4_ARRAY(uint32_t, 0x0063b844);
    const uint32_t da = tabA[i] - tabA[j];
    const uint32_t db = tabB[i] - tabB[j];
    const uint32_t diff = da - db;
    const int64_t prod2 = (int64_t)(int32_t)diff * (int64_t)factor;
    const uint32_t scaled = (uint32_t)((uint64_t)prod2 >> 14);
    const uint32_t res = scaled + db;
    uint32_t fl = (r->eflags & ~0x8D5u) | addFlags(scaled, db, res);
    fl = (fl & ~1u) | ((res >> 31) ^ 1u);
    r->eax = res;
    r->eflags = fl;
}
GP4_IMPL(0x0043c28a, FUN_0043c28a, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0043c28a")
