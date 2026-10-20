#include <gp4/gp4.h>

struct Unknown_00416cc5 {
    uint8_t Unknown00[0x28];
    uint32_t Unknown28;
    uint8_t Unknown2C[0x14];
    int32_t Unknown40;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t comparisonFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00416cc5
// Stores min((|[ebx+0x40]| << 18) / max(DAT_006034a4, (|[ebx+0x40]| >> 14) + 1), 0x400000) at [ebx+0x28].
void __cdecl FUN_00416cc5(gp4::Regs* r) {
    Unknown_00416cc5* body = reinterpret_cast<Unknown_00416cc5*>(static_cast<uintptr_t>(r->ebx));

    uint32_t magnitude = static_cast<uint32_t>(body->Unknown40);
    if (static_cast<int32_t>(magnitude) < 0) magnitude = 0u - magnitude;

    uint32_t high = magnitude >> 14;   // shld edx, eax, 18 with edx = 0
    const uint32_t low = magnitude << 18;
    uint32_t divisor = GP4_GLOBAL(uint32_t, 0x006034a4);
    if (!(divisor > high)) {
        divisor = high + 1u;
        if (divisor == 0u) {
            divisor -= 1u;
            high -= 1u;
        }
    }

    uint32_t remainder;
    uint32_t quotient = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &remainder);

    const uint32_t flags = comparisonFlags32(quotient, 0x400000u);
    if (quotient > 0x400000u) quotient = 0x400000u;
    body->Unknown28 = quotient;

    r->eax = quotient;
    r->edx = remainder;
    r->ebp = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00416cc5, FUN_00416cc5, "regs(ebx:ptr[0x48]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
