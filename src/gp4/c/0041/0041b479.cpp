#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t ParityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (~parity & 1u) << 2;
}

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | ParityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (lhs < rhs ? 1u : 0u) | ParityFlag(result)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Flags left by test/and/or/xor: CF = OF = AF = 0.
static inline uint32_t LogicFlags(uint32_t result) {
    return ParityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7);
}

// FUNCTION: GP4 0x0041b479
void __cdecl FUN_0041b479(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* state = GP4_ARRAY(uint8_t, 0x006380d8);
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t total = GP4_FIELD(uint32_t, state, 0x634) + GP4_FIELD(uint32_t, state, 0x934);
    GP4_GLOBAL(uint32_t, 0x006036ac) = total;

    // First term: round(n1 * max(d1,0)^2 * k1 * k2 * c1), made non-negative.
    int32_t first = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0x86c) - GP4_GLOBAL(uint32_t, 0x00603508));
    if (first < 0) first = 0;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = first;
    Ext t1 = ext_i32(first);
    t1 = t1 * t1;
    t1 = t1 * ext(GP4_GLOBAL(double, 0x006f2b9c));
    t1 = t1 * ext(GP4_GLOBAL(double, 0x006f2b94));
    t1 = t1 * ext_i32(GP4_FIELD(int32_t, body, 0x2f4));
    t1 = t1 * ext(GP4_FIELD(double, state, 0x880));
    const int32_t rounded1 = fistp32(t1);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded1;
    uint32_t magnitude1 = static_cast<uint32_t>(rounded1);
    if (static_cast<int32_t>(magnitude1) < 0) magnitude1 = 0u - magnitude1;

    // Second term, same shape with the other fields.
    int32_t second = static_cast<int32_t>(GP4_FIELD(uint32_t, state, 0xb6c) - GP4_GLOBAL(uint32_t, 0x00603508));
    if (second < 0) second = 0;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = second;
    Ext t2 = ext_i32(second);
    t2 = t2 * t2;
    t2 = t2 * ext(GP4_GLOBAL(double, 0x006f2b9c));
    t2 = t2 * ext(GP4_GLOBAL(double, 0x006f2b94));
    t2 = t2 * ext_i32(GP4_FIELD(int32_t, body, 0x2f8));
    t2 = t2 * ext(GP4_FIELD(double, state, 0xb80));
    const int32_t rounded2 = fistp32(t2);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded2;
    uint32_t magnitude2 = static_cast<uint32_t>(rounded2);
    if (static_cast<int32_t>(magnitude2) < 0) magnitude2 = 0u - magnitude2;

    const uint32_t sum = magnitude2 + magnitude1;
    uint32_t result = sum;
    uint32_t flags = AddFlags(magnitude2, magnitude1, sum);
    if (sum != 0u) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(sum))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060350c));
        result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        if (static_cast<int32_t>(result) > GP4_GLOBAL(int32_t, 0x00603510)) {
            result = GP4_GLOBAL(uint32_t, 0x00603510);
        }
        if (static_cast<int32_t>(total) < 0) {
            flags = LogicFlags(total);
        } else {
            flags = SubtractFlags(0u, result, 0u - result);
            result = 0u - result;
        }
    }
    GP4_GLOBAL(uint32_t, 0x00603514) = result;

    r->eax = result;
    r->ebx = 0x006380d8u;
    r->ebp = magnitude1;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b479, FUN_0041b479, "regs(edx:u32, esi:ptr[0x300]:layout=rec_0041b479) -> (eax, ebx, ebp, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041b479")
