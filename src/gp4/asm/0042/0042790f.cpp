#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// imul + shrd eax, edx, shift: the low dword of the 64-bit product shifted right.
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// FUNCTION: GP4 0x0042790f
// esi = record, ecx = signed input (callers pass |speed|-style values). With
// scale = ([esi+0x284] * [0x603854]) >> 18 (zero becomes 1, made positive) the
// ratio q = (min(|ecx|, scale) << 14) / scale is squared, scaled by [0x62f894]
// and negated into ebp. [esi+0xac] * [0x603854] >> 16 is made positive and
// capped at 0x100000, scaled by [0x6f2bec] >> 14, multiplied by [esi+0x98]; the
// high dword of that >> 6 times ebp is stored at 0x00603d4c. eax = result,
// edx = high dword of the last imul, ebp = the negated squared ratio; the flags
// are those of the final one-operand "imul ebp" (oracle model: CF = OF = the
// product does not fit 32 bits, AF = 0, PF/ZF/SF from the low dword).
void __cdecl FUN_0042790f(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t rate = GP4_GLOBAL(int32_t, 0x00603854);

    // imul [0x603854]; shrd eax, edx, 18
    int32_t scale = static_cast<int32_t>(mulShift(GP4_FIELD(int32_t, rec, 0x284), rate, 0x12));

    int32_t magnitude = static_cast<int32_t>(r->ecx);
    if (magnitude < 0) magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
    if (scale == 0) scale = 1;
    if (scale < 0) scale = static_cast<int32_t>(0u - static_cast<uint32_t>(scale));
    if (magnitude > scale) magnitude = scale;

    // cdq; shld edx, eax, 14; shl eax, 14; idiv ebp
    int32_t remainder = 0;
    const int32_t ratio = gp4::idiv64(static_cast<int64_t>(magnitude) * 16384, scale, &remainder);

    // imul eax; shrd 14; imul [0x62f894]; shrd 14; neg eax
    const uint32_t squared = mulShift(ratio, ratio, 0xe);
    const uint32_t weighted = mulShift(static_cast<int32_t>(squared), GP4_GLOBAL(int32_t, 0x0062f894), 0xe);
    const uint32_t ebp = 0u - weighted;

    // imul [0x603854]; shrd eax, edx, 16; abs; clamp to 0x100000
    int32_t level = static_cast<int32_t>(mulShift(GP4_FIELD(int32_t, rec, 0xac), rate, 0x10));
    if (level < 0) level = static_cast<int32_t>(0u - static_cast<uint32_t>(level));
    if (level > 0x100000) level = 0x100000;

    // imul [0x6f2bec]; shrd 14; imul [esi+0x98]; high dword >> 6
    const uint32_t shifted = mulShift(level, GP4_GLOBAL(int32_t, 0x006f2bec), 0xe);
    const int64_t inner = static_cast<int64_t>(static_cast<int32_t>(shifted)) * GP4_FIELD(int32_t, rec, 0x98);
    const int32_t high = static_cast<int32_t>(static_cast<uint64_t>(inner) >> 32);
    const int32_t sar = high >> 6;

    // imul ebp
    const int64_t product = static_cast<int64_t>(sar) * static_cast<int32_t>(ebp);
    const uint32_t low = static_cast<uint32_t>(product);
    GP4_GLOBAL(uint32_t, 0x00603d4c) = low;

    const bool overflow = product != static_cast<int64_t>(static_cast<int32_t>(low));
    const uint32_t flags = (overflow ? 0x801u : 0u)
        | parityFlag(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low & 0x80000000u) != 0u ? 0x80u : 0u);

    r->eax = low;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042790f, FUN_0042790f, "regs(ecx:i32[-0xffffff..0xffffff], esi:ptr[0x28c]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
