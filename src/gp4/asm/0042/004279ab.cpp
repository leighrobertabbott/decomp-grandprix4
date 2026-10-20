#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right"
static inline uint32_t compare32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, 14 : the low dword of the 64-bit signed product shifted right by 14
static inline uint32_t mulShift14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x004279ab
// pushal/popal wrapper: only the flags of the last flag writer and the dword at [esi+0x284] survive.
// For the segment at edi (below the end pointer 0x891500) the signed bytes at +0x8c of the previous,
// own and next segment are smoothed ((cur + (prev + next) / 2) / 2, *256); a nonzero value is mapped
// through the blend curves 0x6247f0/0x6247f4/0x6247f8 and multiplies the Q14 dword at [esi+0x284].
void __cdecl FUN_004279ab(gp4::Regs* r) {
    const uint32_t seg = r->edi;
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00891500);
    flags = compare32Flags(seg, end);
    if (seg < end) {
        uint8_t* base = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(seg));
        const int32_t cur = GP4_FIELD(int8_t, base, 0x8c);
        const int32_t prev = GP4_FIELD(int8_t, base, -0xd4);
        const int32_t next = GP4_FIELD(int8_t, base, 0x1ec);
        int32_t value = (cur + ((prev + next) >> 1)) >> 1;
        value <<= 8;
        flags = parityFlag(value) | (value == 0 ? 0x40u : 0u) | ((static_cast<uint32_t>(value) >> 24) & 0x80u);
        if (value != 0) {
            uint32_t blend;
            if (value < 0) {
                const int32_t mag = -value;
                const uint32_t low = mulShift14(mag, GP4_GLOBAL(int32_t, 0x006247f0));
                blend = mulShift14(0x4000 - mag, GP4_GLOBAL(int32_t, 0x006247f4)) + low;
            } else {
                int32_t t = 0;
                if (value >= 0x2000) {
                    t = (value - 0x2000) << 1;
                }
                const uint32_t low = mulShift14(t, GP4_GLOBAL(int32_t, 0x006247f8));
                blend = mulShift14(0x4000 - t, GP4_GLOBAL(int32_t, 0x006247f4)) + low;
            }
            // imul edx; shrd eax, edx, 14 (oracle shift model: CF = last bit out, AF = 0,
            // OF = bit31(wide >> 13 xor wide >> 14))
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(blend))
                * static_cast<int64_t>(GP4_FIELD(int32_t, car, 0x284));
            const uint64_t wide = static_cast<uint64_t>(product);
            const uint32_t previous = static_cast<uint32_t>(wide >> 13);
            const uint32_t result = static_cast<uint32_t>(wide >> 14);
            GP4_FIELD(uint32_t, car, 0x284) = result;
            flags = ((previous & 1u) != 0u ? 1u : 0u)
                | parityFlag(result)
                | (result == 0u ? 0x40u : 0u)
                | ((result >> 24) & 0x80u)
                | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004279ab, FUN_004279ab, "regs(edi:rec[0x8943a0,0x160,0..25], esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+bank_004279ab; seed=1")
