#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "sub left, right" / "neg" (left = 0)
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of "test reg, reg" (CF = OF = AF = 0)
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00427880
// ecx = signed input, esi = record. Converts |ecx| << 18 / [0x603854] into a
// quotient, compares it against |[esi+0x284] - quotient| scaled by the larger of
// [esi+0x98] >> 8 and [0x6247b0], and stores a clamped, sign-restored result at
// 0x006247b8. ebp is left holding the signed difference.
void __cdecl FUN_00427880(gp4::Regs* r) {
    const uint8_t* const rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t input = static_cast<int32_t>(r->ecx);
    if (input < 0) input = static_cast<int32_t>(0u - static_cast<uint32_t>(input));

    // cdq; shld edx,eax,18; shl eax,18; idiv [0x603854]
    int32_t remainder = 0;
    const uint32_t scaled = static_cast<uint32_t>(
        gp4::idiv64(static_cast<int64_t>(input) * 262144, GP4_GLOBAL(int32_t, 0x00603854), &remainder));

    const uint32_t diff = GP4_FIELD(uint32_t, const_cast<uint8_t*>(rec), 0x284) - scaled;
    const uint32_t ebp = diff;
    uint32_t magnitude = diff;
    if (static_cast<int32_t>(magnitude) < 0) magnitude = 0u - magnitude;

    int32_t weight = GP4_FIELD(int32_t, const_cast<uint8_t*>(rec), 0x98) >> 8;
    if (weight < GP4_GLOBAL(int32_t, 0x006247b0)) weight = GP4_GLOBAL(int32_t, 0x006247b0);
    if (weight < 0) weight = static_cast<int32_t>(0u - static_cast<uint32_t>(weight));

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(magnitude)) * weight;
    uint32_t low = static_cast<uint32_t>(product);
    uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    uint32_t divisor = scaled;
    if (!(divisor > high)) {
        divisor = high + 1u;
        if (divisor == 0u) {
            divisor = 0xffffffffu;
            high = 0xfffffffeu;
        }
    }
    uint32_t divRemainder = 0;
    uint32_t quotient = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &divRemainder);
    if (quotient > 0x20000000u) quotient = 0x20000000u;

    const int64_t scaledProduct = static_cast<int64_t>(static_cast<int32_t>(quotient)) * GP4_GLOBAL(int32_t, 0x006247ac);
    uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(scaledProduct) >> 14);
    const uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(scaledProduct) >> 32);
    if (static_cast<int32_t>(result) > GP4_GLOBAL(int32_t, 0x006247a8)) {
        result = GP4_GLOBAL(uint32_t, 0x006247a8);
    }

    uint32_t flags;
    if (static_cast<int32_t>(ebp) < 0) {
        flags = subFlags(0u, result);
        result = 0u - result;
    } else {
        flags = logicFlags(ebp);
    }
    GP4_GLOBAL(uint32_t, 0x006247b8) = result;

    r->eax = result;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00427880, FUN_00427880, "regs(ecx:i32[-0xffffff..0xffffff], esi:ptr[0x2a0]:layout=rec_00427880) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00427880")
