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

// Averages a run of signed bytes (cyclic index masked by 0x627b90) of the byte grid at 0x628bc0 from
// offset ebx + edi, scales the sum by 0x627ba4 (>> 14), divides by the run length 0x627ba0, clamps to
// a signed byte and stores it at [ebx + edi + 0x627bbc]. Leaves eax = the clamped value, ecx = the
// run length, edx = the idiv remainder, esi = the cyclic index after the run and the flags of the
// "cmp eax, -0x80" that follows the upper clamp.
// FUNCTION: GP4 0x0041de96
void __cdecl FUN_0041de96(gp4::Regs* r) {
    const uint32_t ebx = r->ebx;
    const uint32_t edi = r->edi;

    int32_t sum = 0;
    uint32_t count = GP4_GLOBAL(uint32_t, 0x00627ba0);
    uint32_t index = ebx;
    do {
        sum += (int32_t)GP4_ARRAY(int8_t, 0x00628bc0)[index + edi];
        index = (index + 1u) & GP4_GLOBAL(uint32_t, 0x00627b90);
        count = count - 1u;
    } while (count != 0u);

    const int64_t product = (int64_t)sum * (int64_t)GP4_GLOBAL(int32_t, 0x00627ba4);
    const int32_t scaled = (int32_t)(uint32_t)((uint64_t)product >> 14);
    const int32_t divisor = (int32_t)GP4_GLOBAL(uint32_t, 0x00627ba0);
    int32_t remainder;
    int32_t value = gp4::idiv64((int64_t)scaled, divisor, &remainder);

    if (!(value <= 0x7f)) {
        value = 0x7f;
    }
    const uint32_t flags = subFlags((uint32_t)value, 0xffffff80u);
    if (!(value >= -0x80)) {
        value = -0x80;
    }
    GP4_ARRAY(int8_t, 0x00627bbc)[ebx + edi] = (int8_t)value;

    r->eax = (uint32_t)value;
    r->ecx = (uint32_t)divisor;
    r->edx = (uint32_t)remainder;
    r->esi = index;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041de96, FUN_0041de96, "regs(ebx:u32[0..63], edi:u32[0..0xfc0]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=grid_00628bc0")
