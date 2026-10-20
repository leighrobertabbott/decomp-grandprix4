#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Mean of two record dwords (add, sar 1) limited to the global `limit`, minus the global `origin`.
static inline int32_t clampedMean(int32_t a, int32_t b, int32_t limit, int32_t origin) {
    int32_t mean = static_cast<int32_t>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b)) >> 1;
    if (mean > limit) mean = limit;
    return static_cast<int32_t>(static_cast<uint32_t>(mean) - static_cast<uint32_t>(origin));
}

// Low half of a signed 64-bit product shifted right by 14 (imul + shrd eax, edx, 14).
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// Computes the two offsets [0x62fc00]/[0x62fc04] of a record's bounds (mean of dwords +0x2fc/+0x300 and
// +0x304/+0x308, limited by [0x6012f8]/[0x6012fc], minus [0x601300]/[0x601304]), scales the first by the
// double [0x6012d8] (fild, fmul, fistp) and the cross sum with [0x601308]/[0x60130c] by the double [0x6012e8], and
// stores both through the Q14 factor in ecx to [0x637f58] and [0x637f5c]. The second imul leaves eax and edx.
// FUNCTION: GP4 0x0040fdfb
void __cdecl FUN_0040fdfb(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t factor = static_cast<int32_t>(r->ecx);

    const int32_t x = clampedMean(GP4_FIELD(int32_t, record, 0x2fc), GP4_FIELD(int32_t, record, 0x300),
                                  GP4_GLOBAL(int32_t, 0x006012f8), GP4_GLOBAL(int32_t, 0x00601300));
    GP4_GLOBAL(int32_t, 0x0062fc00) = x;
    const int32_t z = clampedMean(GP4_FIELD(int32_t, record, 0x304), GP4_FIELD(int32_t, record, 0x308),
                                  GP4_GLOBAL(int32_t, 0x006012fc), GP4_GLOBAL(int32_t, 0x00601304));
    GP4_GLOBAL(int32_t, 0x0062fc04) = z;

    GP4_GLOBAL(int32_t, 0x006f2bf4) = x;
    const int32_t scaledX = gp4::x87::fistp32(gp4::x87::ext_i32(x) * gp4::x87::ext(GP4_GLOBAL(double, 0x006012d8)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaledX;
    GP4_GLOBAL(uint32_t, 0x00637f58) = mulShift14(scaledX, factor);

    const uint32_t fromZ = mulShift14(z, GP4_GLOBAL(int32_t, 0x0060130c));
    const uint32_t fromX = mulShift14(x, GP4_GLOBAL(int32_t, 0x00601308));
    const int32_t sum = static_cast<int32_t>(fromX + fromZ);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = sum;
    const int32_t scaledSum = gp4::x87::fistp32(gp4::x87::ext_i32(sum) * gp4::x87::ext(GP4_GLOBAL(double, 0x006012e8)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaledSum;

    const int64_t product = static_cast<int64_t>(scaledSum) * static_cast<int64_t>(factor);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    GP4_GLOBAL(uint32_t, 0x00637f5c) = result;

    // shrd eax, edx, 14: CF = last bit out, AF = 0 (pinned model), OF = bit31(prev ^ result)
    const uint32_t prev = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    uint32_t flags = parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80000000u ? 0x80u : 0u);
    if ((prev & 1u) != 0u) flags |= 0x01u;
    if (((prev ^ result) & 0x80000000u) != 0u) flags |= 0x800u;

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040fdfb, FUN_0040fdfb, "regs(ecx:i32[-0x80000000..0x7fffffff], esi:ptr[0x310]:layout=car_0040fdfb) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=susp_0040fdfb")
