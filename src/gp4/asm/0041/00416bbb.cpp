#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmp32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_00416bbb {
    uint8_t Unknown00[0x18];
    int32_t Unknown18;
    int32_t Unknown1C;
    uint8_t Unknown20[0x40 - 0x20];
    int32_t Unknown40;
    uint8_t Unknown44[0x17c - 0x44];
    uint32_t Unknown17C;
    int32_t Unknown180;
    uint8_t Unknown184[0x188 - 0x184];
    int32_t Unknown188;
    int32_t Unknown18C;
    uint8_t Unknown190[0x2ac - 0x190];
    int32_t Unknown2AC;
};

// FUNCTION: GP4 0x00416bbb
// Splits a velocity (+0x18, +0x40) into its magnitude and direction ratios: the clamp |[+0x1c]|
// (at least [0x603498]) is stored at +0x2ac and in the global 0x006034a4; +0x180 = the 64-bit
// square root (0x0040113f) of +0x18^2 + +0x40^2; +0x188 = (-[+0x18] << 22) / root and
// +0x18c = ([+0x40] << 22) / root (root 1 when zero); +0x17c = min(0x400000, (|root| << 18) /
// max(clamp, (|root| >> 14) + 1)). eax, ecx (= [+0x40]), edx (division remainder) and ebp (final
// divisor) keep their last values; the flags are those of the final "cmp eax, 0x400000".
void __cdecl FUN_00416bbb(gp4::Regs* r) {
    Unknown_00416bbb* body = reinterpret_cast<Unknown_00416bbb*>(static_cast<uintptr_t>(r->ebx));

    int32_t clamp = body->Unknown1C;
    if (clamp < 0) clamp = static_cast<int32_t>(0u - static_cast<uint32_t>(clamp));
    const int32_t floorValue = GP4_GLOBAL(int32_t, 0x00603498);
    if (clamp < floorValue) clamp = floorValue;
    body->Unknown2AC = clamp;
    GP4_GLOBAL(int32_t, 0x006034a4) = clamp;

    const int32_t across = body->Unknown40;
    const int32_t along = body->Unknown18;
    const uint64_t squares = static_cast<uint64_t>(static_cast<int64_t>(across) * static_cast<int64_t>(across))
        + static_cast<uint64_t>(static_cast<int64_t>(along) * static_cast<int64_t>(along));

    gp4::Regs root = *r;
    root.eax = static_cast<uint32_t>(squares);
    root.edx = static_cast<uint32_t>(squares >> 32);
    gp4::call_regs(0x0040113f, root);
    const uint32_t magnitude = root.eax;
    body->Unknown180 = static_cast<int32_t>(magnitude);

    int32_t divisor = static_cast<int32_t>(magnitude);
    if (divisor == 0) divisor = 1;

    int32_t remainder;
    const int32_t negatedAlong = static_cast<int32_t>(0u - static_cast<uint32_t>(along));
    body->Unknown188 = gp4::idiv64(static_cast<int64_t>(negatedAlong) * 0x400000ll, divisor, &remainder);
    body->Unknown18C = gp4::idiv64(static_cast<int64_t>(across) * 0x400000ll, divisor, &remainder);

    uint32_t absRoot = body->Unknown180 < 0 ? 0u - static_cast<uint32_t>(body->Unknown180) : static_cast<uint32_t>(body->Unknown180);
    uint32_t high = absRoot >> 14;
    const uint32_t low = absRoot << 18;
    uint32_t bound = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006034a4));
    if (!(bound > high)) {
        bound = high + 1u;
        if (bound == 0u) {
            --bound;
            --high;
        }
    }
    uint32_t rest;
    const uint32_t quotient = gp4::div64((static_cast<uint64_t>(high) << 32) | low, bound, &rest);
    body->Unknown17C = quotient > 0x400000u ? 0x400000u : quotient;

    r->eax = body->Unknown17C;
    r->ecx = static_cast<uint32_t>(across);
    r->edx = rest;
    r->ebp = bound;
    r->eflags = (r->eflags & ~0x8d5u) | cmp32Flags(quotient, 0x400000u);
}
GP4_IMPL(0x00416bbb, FUN_00416bbb, "regs(ebx:ptr[0x2b4]:layout=vec_00416bbb) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=floor_00603498")
