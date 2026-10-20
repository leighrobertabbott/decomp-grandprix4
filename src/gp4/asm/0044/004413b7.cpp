#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t resultFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | resultFlags(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of INC before (carry kept from the previous writer).
static inline uint32_t incFlags32(uint32_t before, uint32_t carry) {
    const uint32_t result = before + 1u;
    return carry | resultFlags(result)
        | ((before & 0xfu) == 0xfu ? 0x10u : 0u)
        | (result == 0x80000000u ? 0x800u : 0u);
}

// Flags of DEC before (carry kept from the previous writer).
static inline uint32_t decFlags32(uint32_t before, uint32_t carry) {
    const uint32_t result = before - 1u;
    return carry | resultFlags(result)
        | ((before & 0xfu) == 0u ? 0x10u : 0u)
        | (before == 0x80000000u ? 0x800u : 0u);
}

// One scaled quotient: ((sample << 14) + [0x6020a8]) * multiplier / divisor with a 64/32 DIV. The divisor
// (edi, carried from the previous call) is first raised to edx + 1 when it is not above the product's high
// dword (and the pair is pulled down by one when edx + 1 wraps), so the quotient always fits. `flags` is
// updated to the flags the original leaves (DIV keeps those of the last cmp/inc/dec).
static inline uint32_t scaledQuotient(uint32_t sample, uint32_t multiplier, uint32_t* divisor, uint32_t* flags) {
    const uint32_t scaled = (sample << 14) + GP4_GLOBAL(uint32_t, 0x006020a8);
    const uint64_t product = static_cast<uint64_t>(scaled) * multiplier;
    uint32_t low = static_cast<uint32_t>(product);
    uint32_t high = static_cast<uint32_t>(product >> 32);
    uint32_t edi = *divisor;

    uint32_t f = cmpFlags32(edi, high);
    if (!(edi > high)) {
        const uint32_t original = high;
        edi = high + 1u;
        f = incFlags32(original, f & 1u);
        if (edi == 0u) {
            f = decFlags32(0u, f & 1u);
            edi = 0xffffffffu;
            f = decFlags32(original, f & 1u);
            high = original - 1u;
        }
    }
    *divisor = edi;
    *flags = f;
    uint32_t remainder;
    return gp4::div64((static_cast<uint64_t>(high) << 32) | low, edi, &remainder);
}

// Fills the four scaled ratios at +0x188/+0x18c/+0x190/+0x194 of the record in esi: the row is
// (byte +0x7c & 0x3f) - 1 clamped to 0x15, the column sel = byte [0x66d936 + row], the multiplier the dword
// [0x66ec60 + row*0xb0 + sel*4], each ratio from one of the byte tables 0x66dd2a / 0x66e0f2 / 0x66e4ba /
// 0x66e882 at [row*0x2c + sel], divided by 0xf0 * [0x602a9c]. pushal/popal keep every register; the flags
// are those the last DIV inherited.
// FUNCTION: GP4 0x004413b7
void __cdecl FUN_004413b7(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t row = static_cast<int32_t>(rec[0x7c] & 0x3fu) - 1;
    if (row > 0x15) {
        row = 0x15;
    }
    const int32_t sel = GP4_ARRAY(uint8_t, 0x0066d936)[row];
    const int32_t cell = row * 0x2c + sel;
    const uint32_t multiplier = GP4_ARRAY(uint32_t, 0x0066ec60)[cell];

    uint32_t divisor = 0xf0u * GP4_GLOBAL(uint32_t, 0x00602a9c);
    uint32_t flags = 0u;
    const uint32_t q0 = scaledQuotient(GP4_ARRAY(uint8_t, 0x0066dd2a)[cell], multiplier, &divisor, &flags);
    GP4_FIELD(uint32_t, rec, 0x188) = q0;
    const uint32_t q1 = scaledQuotient(GP4_ARRAY(uint8_t, 0x0066e0f2)[cell], multiplier, &divisor, &flags);
    GP4_FIELD(uint32_t, rec, 0x18c) = q1;
    const uint32_t q2 = scaledQuotient(GP4_ARRAY(uint8_t, 0x0066e4ba)[cell], multiplier, &divisor, &flags);
    GP4_FIELD(uint32_t, rec, 0x190) = q2;
    const uint32_t q3 = scaledQuotient(GP4_ARRAY(uint8_t, 0x0066e882)[cell], multiplier, &divisor, &flags);
    GP4_FIELD(uint32_t, rec, 0x194) = q3;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004413b7, FUN_004413b7, "regs(esi:ptr[0x19c]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=counter_0040f0dd+tabs_004413b7")
