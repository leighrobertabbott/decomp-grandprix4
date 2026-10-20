#include <gp4/gp4.h>

struct Unknown_00419b84 {
    uint8_t Unknown00[0xb2];
    int16_t UnknownB2;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "add left, right" (dword)
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// flags of "cmp byte, 0": CF = OF = AF = 0, the rest follow the byte itself
static inline uint32_t cmpByteZeroFlags(uint8_t value) {
    return parityFlag(value) | (value == 0 ? 0x40u : 0u) | ((value & 0x80u) != 0 ? 0x80u : 0u);
}

// Sum of the +0x3c dword of the four 0x300-byte records at 0x006380d8.
static __declspec(noinline) int32_t slotSum(uint8_t* records) {
    uint32_t sum = (uint32_t)GP4_FIELD(int32_t, records, 0x3c);
    sum += (uint32_t)GP4_FIELD(int32_t, records, 0x33c);
    sum += (uint32_t)GP4_FIELD(int32_t, records, 0x63c);
    sum += (uint32_t)GP4_FIELD(int32_t, records, 0x93c);
    return (int32_t)sum;
}

// (value << 3) sign-extended to 64 bits, shifted left by 8 (cdq; shld edx,eax,8; shl eax,8),
// divided by the divisor with a single idiv.
static inline int32_t scaledQuotient(int32_t value, int32_t divisor, int32_t* remainder) {
    const int32_t shifted = (int32_t)((uint32_t)value << 3);
    return gp4::idiv64((int64_t)shifted << 8, divisor, remainder);
}

// Sums the four per-slot values (+0x3c of each record) with [0x637fac] into [0x637fd0], then
// divides both [0x637fc8] (cleared to zero first) and that sum by max(word at +0xb2, 0x200) in
// 24.8 fixed point; the results go to [0x637f60] (plus [0x637f98]) and [0x637f64] / [0x638cec]
// (plus [0x637fa0] unless the byte [0x622485] is set).
// FUNCTION: GP4 0x00419b84
void __cdecl FUN_00419b84(gp4::Regs* r) {
    const Unknown_00419b84* body = reinterpret_cast<const Unknown_00419b84*>(r->esi);
    uint8_t* records = GP4_ARRAY(uint8_t, 0x006380d8);

    GP4_GLOBAL(int32_t, 0x00637fc8) = 0;
    const int32_t total = (int32_t)((uint32_t)slotSum(records) + (uint32_t)GP4_GLOBAL(int32_t, 0x00637fac));
    GP4_GLOBAL(int32_t, 0x00637fd0) = total;

    int32_t divisor = body->UnknownB2;
    if (divisor < 0x200) {
        divisor = 0x200;
    }

    int32_t remainder;
    int32_t quotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fc8), divisor, &remainder);
    GP4_GLOBAL(int32_t, 0x00637f60) = (int32_t)((uint32_t)quotient + (uint32_t)GP4_GLOBAL(int32_t, 0x00637f98));

    quotient = scaledQuotient(GP4_GLOBAL(int32_t, 0x00637fd0), divisor, &remainder);
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00622485);
    uint32_t flags = cmpByteZeroFlags(mode);
    if (mode == 0) {
        const int32_t offset = GP4_GLOBAL(int32_t, 0x00637fa0);
        flags = addFlags((uint32_t)quotient, (uint32_t)offset);
        quotient = (int32_t)((uint32_t)quotient + (uint32_t)offset);
    }
    GP4_GLOBAL(int32_t, 0x00637f64) = quotient;
    GP4_GLOBAL(int32_t, 0x00638cec) = quotient;

    r->eax = (uint32_t)quotient;
    r->ebx = 0x006380d8u;
    r->ecx = (uint32_t)divisor;
    r->edx = (uint32_t)remainder;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419b84, FUN_00419b84, "regs(esi:ptr[0xba]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sums_00419f9e")
