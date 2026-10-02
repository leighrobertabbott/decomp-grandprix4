#include <gp4/gp4.h>

static inline uint32_t logicalFlags(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0041de65
void __cdecl FUN_0041de65(gp4::Regs* r) {
    const int32_t first = GP4_GLOBAL(int32_t, 0x00623e30);
    const int32_t second = GP4_GLOBAL(int32_t, 0x00623e38);
    const uint32_t mask = GP4_GLOBAL(uint32_t, 0x00627b90);
    const uint32_t shift = GP4_GLOBAL(uint32_t, 0x00627b8c);
    const uint32_t firstCoordinate = static_cast<uint32_t>(first >> 17) & mask;
    const uint32_t secondCoordinate = (static_cast<uint32_t>(second >> 17) & mask) << (shift & 31u);
    const uint32_t index = firstCoordinate | secondCoordinate;
    const int32_t value = GP4_ARRAY(int8_t, 0x00627bbc)[index];
    r->eax = static_cast<uint32_t>(value);
    r->edx = secondCoordinate;
    r->ecx = shift;
    r->eflags = (r->eflags & ~0x8d5u) | logicalFlags(index);
}
GP4_IMPL(0x0041de65, FUN_0041de65, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=table_00627bbc")
