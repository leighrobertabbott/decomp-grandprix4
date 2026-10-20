#include <gp4/gp4.h>

struct Unknown_0041d0e8 {
    uint8_t Unknown000[0x1c0];
    uint16_t Unknown1c0;
    uint16_t Unknown1c2;
    uint8_t Unknown1c4[0x37c - 0x1c4];
    int32_t Unknown37c;
    int32_t Unknown380;
    int32_t Unknown384;
    int32_t Unknown388;
    int32_t Unknown38c;
    int32_t Unknown390;
    int32_t Unknown394;
    int32_t Unknown398;
};

// neg / cmp 0x80000 / cmov-style clamp of the original: result limited to -0x80000..0x80000.
static inline int32_t clampTo19Bits(int32_t v) {
    if (v >= 0) {
        if (static_cast<uint32_t>(v) >= 0x80000u) v = 0x80000;
        return v;
    }
    uint32_t m = 0u - static_cast<uint32_t>(v);
    if (m >= 0x80000u) m = 0x80000u;
    return static_cast<int32_t>(0u - m);
}

// FUNCTION: GP4 0x0041d0e8
// pushal/popal routine over the record at esi: from the eight dwords at +0x37c..+0x398 it forms the
// two half-sums, the difference of the diagonal sums and the two cross differences (each clamped to
// +-0x80000), scales them by the gains 0x00601350 / 0x0060134c (divided by 0x00601334) and the
// divisors 0x00638da4+0x00638da8 / 0x00638dac / 0x00638db0 and accumulates the results >> 16 into
// the words at +0x1c0 and +0x1c2. Registers are restored; the flags are those of the final
// `add word [esi+0x1c2], ax`.
void __cdecl FUN_0041d0e8(gp4::Regs* r) {
    Unknown_0041d0e8* rec = reinterpret_cast<Unknown_0041d0e8*>(static_cast<uintptr_t>(r->esi));

    // First term: ((sum of the 0x384 group) >> 1) - ((sum of the 0x37c group) >> 1).
    const int32_t halfA = static_cast<int32_t>(static_cast<uint32_t>(rec->Unknown37c) + static_cast<uint32_t>(rec->Unknown38c) +
                                               static_cast<uint32_t>(rec->Unknown380) + static_cast<uint32_t>(rec->Unknown390)) >> 1;
    const int32_t halfB = static_cast<int32_t>(static_cast<uint32_t>(rec->Unknown384) + static_cast<uint32_t>(rec->Unknown394) +
                                               static_cast<uint32_t>(rec->Unknown388) + static_cast<uint32_t>(rec->Unknown398)) >> 1;
    const int32_t diff1 = clampTo19Bits(static_cast<int32_t>(static_cast<uint32_t>(halfB) - static_cast<uint32_t>(halfA)));
    const int32_t divisor1 = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00638da8) + GP4_GLOBAL(uint32_t, 0x00638da4));
    int32_t rem;
    const int32_t quot1 = gp4::idiv64(static_cast<int64_t>(diff1) << 26, divisor1, &rem);
    rec->Unknown1c0 = static_cast<uint16_t>(rec->Unknown1c0 + static_cast<uint16_t>(quot1 >> 16));

    // Second term.
    const int32_t diff2 = clampTo19Bits(static_cast<int32_t>(static_cast<uint32_t>(rec->Unknown37c) + static_cast<uint32_t>(rec->Unknown38c) -
                                                             static_cast<uint32_t>(rec->Unknown380) - static_cast<uint32_t>(rec->Unknown390)));
    const int32_t scaled2 = gp4::idiv64(static_cast<int64_t>(diff2) * GP4_GLOBAL(int32_t, 0x00601350), GP4_GLOBAL(int32_t, 0x00601334), &rem);
    const int32_t part2 = gp4::idiv64(static_cast<int64_t>(scaled2) << 25, GP4_GLOBAL(int32_t, 0x00638dac), &rem);

    // Third term.
    const int32_t diff3 = clampTo19Bits(static_cast<int32_t>(static_cast<uint32_t>(rec->Unknown384) + static_cast<uint32_t>(rec->Unknown394) -
                                                             static_cast<uint32_t>(rec->Unknown388) - static_cast<uint32_t>(rec->Unknown398)));
    const int32_t scaled3 = gp4::idiv64(static_cast<int64_t>(diff3) * GP4_GLOBAL(int32_t, 0x0060134c), GP4_GLOBAL(int32_t, 0x00601334), &rem);
    const int32_t part3 = gp4::idiv64(static_cast<int64_t>(scaled3) << 25, GP4_GLOBAL(int32_t, 0x00638db0), &rem);

    const uint32_t addend = static_cast<uint32_t>(static_cast<int32_t>(static_cast<uint32_t>(part3) + static_cast<uint32_t>(part2)) >> 16) & 0xffffu;
    const uint32_t oldWord = rec->Unknown1c2;
    const uint32_t sum = oldWord + addend;
    rec->Unknown1c2 = static_cast<uint16_t>(sum);

    // Flags of the 16-bit add.
    const uint32_t result = sum & 0xffffu;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    uint32_t flags = 0;
    flags |= (sum >> 16) & 1u;
    flags |= (parity & 1u) == 0u ? 4u : 0u;
    flags |= (oldWord ^ addend ^ result) & 0x10u;
    flags |= result == 0u ? 0x40u : 0u;
    flags |= (result >> 8) & 0x80u;
    flags |= (((oldWord ^ result) & (addend ^ result)) & 0x8000u) ? 0x800u : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041d0e8, FUN_0041d0e8, "regs(esi:ptr[0x3a0]:i32[-1048576..1048576]) -> (cf, pf, af, zf, sf, of); globals=layout=recs_006380d8")
