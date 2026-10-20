#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `left + right` (32-bit).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a logic op (test) whose result is `value`: CF = OF = AF = 0.
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// The clamp limit 0x580000 is built at run time: as an immediate it would read as a code address.
static __declspec(noinline) uint32_t clampLimit(uint32_t half) {
    return half * 2u;
}

// Clamp the magnitude of a signed value to 0x580000, keeping its sign.
static inline int32_t clampMagnitude(int32_t value) {
    const uint32_t limit = clampLimit(0x2c0000u);
    if (value < 0) {
        uint32_t magnitude = 0u - (uint32_t)value;
        if (magnitude >= limit) magnitude = limit;
        return (int32_t)(0u - magnitude);
    }
    if ((uint32_t)value >= limit) return (int32_t)limit;
    return value;
}

// Adds one of four clamped car dwords (+0x39c/+0x3a0/+0x3a4/+0x3a8 of esi), scaled by
// ([0x6f2bf0] << 8) / [0x6013d8] and negated, to the accumulator 0x623e18. Which dword is chosen by the
// mask dword +0x4c of the record in ebx: bit 0x10000 only (or with 0x8000), 0x4000 (or 0x2000),
// 0x200 (or 0x100), 0x80 (or 0x40).
// FUNCTION: GP4 0x004248af
void __cdecl FUN_004248af(gp4::Regs* r) {
    uint8_t* obj = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t mask = GP4_FIELD(uint32_t, obj, 0x4c);

    uint32_t offset;
    if ((mask & 0x10000u) != 0u && (mask & 0xfffe7fffu) == 0u) {
        offset = 0x39c;
    } else if ((mask & 0x4000u) != 0u && (mask & 0xffff9fffu) == 0u) {
        offset = 0x3a0;
    } else if ((mask & 0x200u) != 0u && (mask & 0xfffffcffu) == 0u) {
        offset = 0x3a4;
    } else if ((mask & 0x80u) != 0u && (mask & 0xffffff3fu) == 0u) {
        offset = 0x3a8;
    } else {
        // No case: the flags are those of the last test (bit 0x80, then the remaining bits).
        const uint32_t last = (mask & 0x80u) == 0u ? 0u : (mask & 0xffffff3fu);
        r->ebp = mask;
        r->eflags = (r->eflags & ~0x8d5u) | testFlags32(last);
        return;
    }

    const int32_t value = clampMagnitude(GP4_FIELD(int32_t, car, offset));
    const int32_t scale = (int32_t)(GP4_GLOBAL(uint32_t, 0x006f2bf0) << 8);
    const int64_t product = (int64_t)value * (int64_t)scale;
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(product, GP4_GLOBAL(int32_t, 0x006013d8), &remainder);
    const uint32_t negated = 0u - (uint32_t)quotient;

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00623e18);
    GP4_GLOBAL(uint32_t, 0x00623e18) = before + negated;

    r->eax = negated;
    r->edx = (uint32_t)remainder;
    r->ebp = mask;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, negated);
}
GP4_IMPL(0x004248af, FUN_004248af, "regs(ebx:ptr[0x75]:layout=rec_0042453d, esi:ptr[0x3b0]:i32[-8000000..8000000]) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041bce9")
