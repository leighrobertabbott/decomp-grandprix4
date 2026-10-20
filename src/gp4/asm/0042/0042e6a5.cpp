#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of the 32-bit "add left, right".
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 0x1u : 0u);
}

// FUNCTION: GP4 0x0042e6a5
void __cdecl FUN_0042e6a5(gp4::Regs* r) {
    const uint8_t* ebx = (const uint8_t*)r->ebx;
    uint8_t* esi = (uint8_t*)r->esi;

    // word [esi+0x1bc] = ebx - 0x66fef0
    GP4_FIELD(uint16_t, esi, 0x1bc) = (uint16_t)(r->ebx - 0x0066fef0u);

    // edx:eax = (int16)[esi+0x9a] * [0x604284]; shrd eax, edx, 14 (edx keeps the high half)
    const int64_t product = (int64_t)*(const int16_t*)(esi + 0x9a) * (int64_t)GP4_GLOBAL(int32_t, 0x00604284);
    uint32_t value = (uint32_t)((uint64_t)product >> 14);
    const uint32_t high = (uint32_t)((uint64_t)product >> 32);

    value = 0u - value;
    value = value + GP4_GLOBAL(uint32_t, 0x00604288);
    if ((ebx[0x161] & 0x80u) != 0u) {
        value = value + GP4_GLOBAL(uint32_t, 0x00603b88);
    }
    if ((int32_t)value < GP4_GLOBAL(int32_t, 0x0060428c)) {
        value = GP4_GLOBAL(uint32_t, 0x0060428c);
    }

    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_FIELD(uint32_t, esi, 0x1d4) = base;
    GP4_FIELD(uint32_t, esi, 0x1d4) = GP4_FIELD(uint32_t, esi, 0x1d4) + value;

    r->eax = value;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(base, value);
}
GP4_IMPL(0x0042e6a5, FUN_0042e6a5, "regs(ebx:ptr[0x169]:bytes, esi:ptr[0x1dc]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
