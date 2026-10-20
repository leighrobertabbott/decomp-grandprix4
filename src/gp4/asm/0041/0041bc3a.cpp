#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of "neg value".
static inline uint32_t negationFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

struct Unknown_0041bc3a_Ebx {
    uint8_t Unknown00[0x25c];
    int32_t Unknown25C;
    uint8_t Unknown260[0x55c - 0x260];
    int32_t Unknown55C;
};

// FUNCTION: GP4 0x0041bc3a
// Weighted mean of two terms stored to DAT_006258b0 and, clamped to +-DAT_006258b4, to DAT_006258ac
// (DAT_006258dc = 0xff when the clamp applied, else 0):
//   q1 = ((ebx[0x25c] + ebx[0x55c]) << 14) / DAT_00623b6c         (ecx = high dword of q1 * DAT_00623b0c)
//   v  = DAT_0062474c, scaled by DAT_0062fb54 (Q14) when byte DAT_0062fb4a == 0
//   q2 = (v * (2*DAT_00623af4 + DAT_00623b10) - q1 * DAT_00623b0c) / (2*DAT_00623af4 + DAT_00623b0c + DAT_00623b10)
// eax = q2 or the signed clamp, edx = |q2|, ebp = the divisor. The flags are those of the last cmp,
// or of the test/neg of the clamp path.
void __cdecl FUN_0041bc3a(gp4::Regs* r) {
    const Unknown_0041bc3a_Ebx* body = reinterpret_cast<const Unknown_0041bc3a_Ebx*>(static_cast<uintptr_t>(r->ebx));

    const int32_t sum = static_cast<int32_t>(static_cast<uint32_t>(body->Unknown25C) + static_cast<uint32_t>(body->Unknown55C));
    int32_t remainder;
    const int32_t q1 = gp4::idiv64(static_cast<int64_t>(sum) << 14, GP4_GLOBAL(int32_t, 0x00623b6c), &remainder);

    const int64_t weighted = static_cast<int64_t>(q1) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00623b0c));

    uint32_t value = GP4_GLOBAL(uint32_t, 0x0062474c);
    if (GP4_GLOBAL(uint8_t, 0x0062fb4a) == 0) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062fb54));
        const uint32_t low = static_cast<uint32_t>(product);
        const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        value = (low >> 14) | (high << 18);
    }

    const uint32_t weightA = (static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623af4)) << 1) + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623b10));
    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(value)) * static_cast<int64_t>(static_cast<int32_t>(weightA));
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(first) - static_cast<uint64_t>(weighted));

    const uint32_t divisor = (static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623af4)) << 1)
        + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623b0c)) + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623b10));
    const int32_t q2 = gp4::idiv64(dividend, static_cast<int32_t>(divisor), &remainder);
    GP4_GLOBAL(int32_t, 0x006258b0) = q2;

    int32_t magnitude = q2;
    if (q2 < 0) {
        magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(q2));
    }
    const int32_t limit = GP4_GLOBAL(int32_t, 0x006258b4);

    uint32_t result;
    uint32_t flags = subtractionFlags32(static_cast<uint32_t>(magnitude), static_cast<uint32_t>(limit));
    if (magnitude < limit) {
        GP4_GLOBAL(int32_t, 0x006258ac) = q2;
        GP4_GLOBAL(uint8_t, 0x006258dc) = 0;
        result = static_cast<uint32_t>(q2);
    } else {
        const int32_t stored = GP4_GLOBAL(int32_t, 0x006258b0);
        result = static_cast<uint32_t>(limit);
        // test dword [0x6258b0], -1
        flags = parityFlag(static_cast<uint32_t>(stored))
            | (stored == 0 ? 0x40u : 0u)
            | ((static_cast<uint32_t>(stored) & 0x80000000u) != 0u ? 0x80u : 0u);
        if (stored < 0) {
            flags = negationFlags32(result);
            result = 0u - result;
        }
        GP4_GLOBAL(int32_t, 0x006258ac) = static_cast<int32_t>(result);
        GP4_GLOBAL(uint8_t, 0x006258dc) = 0xff;
    }

    r->eax = result;
    r->ecx = static_cast<uint32_t>(static_cast<uint64_t>(weighted) >> 32);
    r->edx = static_cast<uint32_t>(magnitude);
    r->ebp = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041bc3a, FUN_0041bc3a, "regs(ebx:ptr[0x564]:layout=rec_0041bbef) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0041bbef+state_0041bd28+state_0041b950+div_0041bc3a")
