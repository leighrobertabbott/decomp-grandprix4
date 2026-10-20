#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Signed 64/32 division as IDIV performs it (truncating toward zero, remainder
// takes the dividend's sign), built from 32-bit steps so no CRT helper is needed.
static inline void idiv64(uint32_t numHi, uint32_t numLo, int32_t divisor,
                          int32_t* quotient, int32_t* remainder) {
    const bool numNegative = (numHi & 0x80000000u) != 0u;
    const bool divNegative = divisor < 0;
    if (numNegative) {
        numLo = 0u - numLo;
        numHi = ~numHi + (numLo == 0u ? 1u : 0u);
    }
    const uint32_t d = divNegative ? (0u - (uint32_t)divisor) : (uint32_t)divisor;

    uint32_t rem = 0u;
    uint32_t quo = 0u;
    for (int i = 0; i < 64; ++i) {
        const uint32_t top = rem >> 31;
        rem = (rem << 1) | (numHi >> 31);
        numHi = (numHi << 1) | (numLo >> 31);
        numLo <<= 1;
        quo <<= 1;
        if (top != 0u || rem >= d) {
            rem -= d;
            quo |= 1u;
        }
    }
    *quotient = (numNegative != divNegative) ? (int32_t)(0u - quo) : (int32_t)quo;
    *remainder = numNegative ? (int32_t)(0u - rem) : (int32_t)rem;
}

// FUNCTION: GP4 0x0041246e
void __cdecl FUN_0041246e(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;
    const int32_t scale = (int32_t)GP4_FIELD(int16_t, body, 0xe0);

    if (scale == 0) {
        GP4_GLOBAL(int32_t, 0x00601c70) = 0;
        r->eax = 0;
        // or eax,eax on zero: ZF and PF set, CF/OF/SF/AF clear
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    const int32_t factorA = (int32_t)GP4_FIELD(uint16_t, body, 0xd6);
    const int32_t factorB = (int32_t)GP4_FIELD(uint16_t, body, 0xb2);

    // imul edx / shrd eax,edx,14 : (scale * factorA) >> 14, low 32 bits kept
    const int64_t product = (int64_t)scale * (int64_t)factorA;
    const int32_t scaled = (int32_t)(((uint32_t)product >> 14) | ((uint32_t)((uint64_t)product >> 32) << 18));

    // imul edx / shld+shl by 3 : 64-bit numerator
    const int64_t numerator = ((int64_t)scaled * (int64_t)factorB) << 3;
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x00603854);
    int32_t quotient;
    int32_t remainder;
    idiv64((uint32_t)((uint64_t)numerator >> 32), (uint32_t)numerator, divisor, &quotient, &remainder);

    // neg eax
    const uint32_t before = (uint32_t)quotient;
    const uint32_t result = 0u - before;
    const uint32_t flags = (before != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((before ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (before == 0x80000000u ? 0x800u : 0u);

    GP4_GLOBAL(uint32_t, 0x00601c70) = result;
    r->eax = result;
    r->edx = (uint32_t)remainder;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041246e, FUN_0041246e, "regs(esi:ptr[0xe8]:layout=rec_0041246e) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041246e")
