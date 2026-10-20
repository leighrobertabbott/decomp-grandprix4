#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of the 32-bit "cmp left, right".
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 0x1u : 0u);
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

// FUNCTION: GP4 0x0041adbd
void __cdecl FUN_0041adbd(gp4::Regs* r) {
    const uint8_t* body = (const uint8_t*)r->esi;

    int32_t group = (int32_t)(body[0x7c] & 0x3fu) - 1;
    if (!(group <= 0x15)) {
        group = 0x15;
    }
    const int32_t rowOffset = group << 4;

    int32_t step = (int32_t)(int8_t)body[0x72];
    if (step < 0) {
        step = -step;
    }

    uint32_t flags = subFlags((uint32_t)step, 2u);
    uint32_t result;
    if (step < 2) {
        result = GP4_GLOBAL(uint32_t, 0x00601314);
    } else {
        const int32_t limit = (int32_t)body[0x1cd];
        flags = subFlags((uint32_t)step, (uint32_t)limit);
        if (!(step <= limit)) {
            step = limit;
        }
        const int16_t value = *(const int16_t*)(GP4_ARRAY(const uint8_t, 0x007c28bc) + rowOffset + step * 2);
        const uint32_t base = (uint32_t)(int32_t)value;
        const uint32_t addend = GP4_GLOBAL(uint32_t, 0x00601324);
        result = base + addend;
        flags = addFlags(base, addend);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041adbd, FUN_0041adbd, "regs(esi:ptr[0x1d5]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
