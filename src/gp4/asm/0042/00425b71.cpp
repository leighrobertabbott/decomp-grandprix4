#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `left - right` (cmp).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00425b71
// Writes the car record word +0x13c from the signed 16-bit value in ax (x):
//   x >= 0x6000          -> word +0x13c = 0
//   otherwise            -> blend = (rnd8 << 6) * (w+0x1ac - w+0x108) >> 14 + w+0x108, where the random
//                           byte comes from 0x00401462; unless byte +0x1f5 is non-zero and still non-zero
//                           after its decrement, blend is replaced by the result of 0x0042595c;
//                           then ((blend + 0x4000) * x >> 14) is capped at 0x7700 and word +0x13c = that - x.
// Registers left: eax = capped value (or x when x >= 0x6000), ecx = word +0x108 (or the 0x0042595c
// output), edx = value - x, ebp as left by 0x0042595c, flags of the last add (or of cmp x, 0x6000).
void __cdecl FUN_00425b71(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t x = static_cast<int32_t>(static_cast<int16_t>(r->eax));

    if (x >= 0x6000) {
        GP4_FIELD(uint16_t, record, 0x13c) = 0;
        r->eax = static_cast<uint32_t>(x);
        r->eflags = (r->eflags & ~0x8d5u) | subFlags32(static_cast<uint32_t>(x), 0x6000u);
        return;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = static_cast<uint32_t>(x);
    gp4::call_regs(0x00401462, call);
    const int32_t scaledRandom = static_cast<int32_t>((call.eax & 0xffu) << 6);

    const int32_t upper = GP4_FIELD(const int16_t, record, 0x1ac);
    const int32_t base = GP4_FIELD(const int16_t, record, 0x108);
    const int64_t product = static_cast<int64_t>(scaledRandom) * static_cast<int64_t>(upper - base);
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    uint32_t ecx = static_cast<uint32_t>(base);
    uint32_t ebp = r->ebp;
    eax += ecx;

    bool callPolicy = true;
    if (GP4_FIELD(const uint8_t, record, 0x1f5) != 0u) {
        const uint8_t remaining = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x1f5) - 1u);
        GP4_FIELD(uint8_t, record, 0x1f5) = remaining;
        callPolicy = (remaining == 0u);
    }
    if (callPolicy) {
        call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = eax;
        call.ecx = ecx;
        call.edx = edx;
        gp4::call_regs(0x0042595c, call);
        eax = call.eax;
        ecx = call.ecx;
        ebp = call.ebp;
    }

    eax += 0x4000u;
    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(x);
    eax = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
    if (static_cast<int32_t>(eax) >= 0x7700) {
        eax = 0x7700u;
    }
    const uint32_t negated = 0u - static_cast<uint32_t>(x);
    edx = negated + eax;
    GP4_FIELD(uint16_t, record, 0x13c) = static_cast<uint16_t>(edx);

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(negated, eax);
}
GP4_IMPL(0x00425b71, FUN_00425b71, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x423]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042595c")
