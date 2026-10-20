#include <gp4/gp4.h>

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (value * gain) >> 24 as the low dword of `imul dword [gain]; shrd eax, edx, 24`.
static inline int32_t scaleByGain(int32_t value, int32_t gain) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(gain);
    return static_cast<int32_t>(product >> 24);
}

// esi = car record. Runs the force solver 0x004057cf (with [0x006030d0] = -[0x00604124]) and the
// smoothing 0x004279ab (esi car, edi segment), then returns in eax the car's Q14 factor +0x284
// (scaled by [0x00603854] >> 24) divided by the car's +0x98 value likewise scaled (floored at
// 0x1000), as (factor << 18) / divisor capped at 0x4000; edx = the idiv remainder. The dword +0x284
// is restored to its entry value; the flags are those of `cmp eax, 0x4000`.
// FUNCTION: GP4 0x004054e6
void __cdecl FUN_004054e6(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedFactor = GP4_FIELD(uint32_t, car, 0x284);

    GP4_GLOBAL(int32_t, 0x006030d0) = -GP4_GLOBAL(int32_t, 0x00604124);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004057cf, call);
    gp4::call_regs(0x004279ab, call);

    const int32_t gain = GP4_GLOBAL(int32_t, 0x00603854);

    int32_t divisor = scaleByGain(GP4_FIELD(int32_t, car, 0x98), gain);
    if (divisor < 0x1000) {
        divisor = 0x1000;
    }

    const int32_t factor = scaleByGain(GP4_FIELD(int32_t, car, 0x284), gain);
    int32_t remainder = 0;
    int32_t quotient = gp4::idiv64(static_cast<int64_t>(factor) << 18, divisor, &remainder);

    const uint32_t flags = cmpFlags32(static_cast<uint32_t>(quotient), 0x4000u);
    if (quotient > 0x4000) {
        quotient = 0x4000;
    }

    GP4_FIELD(uint32_t, car, 0x284) = savedFactor;

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004054e6, FUN_004054e6, "regs(esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+bank_004279ab+state_00405489+engine_00413606+drive_00405552+seg_00405552+torque_00425e19+state_0041062a+drive_004054e6")
