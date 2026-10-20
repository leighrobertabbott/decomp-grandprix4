#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t flags = parityFlag8(result);
    if ((result & 0xffu) == 0u) flags |= 0x40u;
    if ((result & 0x80u) != 0u) flags |= 0x80u;
    return flags;
}

// Arithmetic flags of the 16-bit `left + right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    const uint32_t sum = left + right;
    const uint32_t result = sum & 0xffffu;
    return (sum > 0xffffu ? 1u : 0u) | parityFlag8(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// ax = word[edi+0x16] + (((word[edi+0x176] - word[edi+0x16]) * word[esi+0x66]) >> 14), the high
// 16 bits of the product scaled by 4 (the two shl/rcl pairs); 0 when esi's flag 0x9c&0x10 is set.
// dx is left as the shifted high half.
// FUNCTION: GP4 0x004030fe
void __cdecl FUN_004030fe(gp4::Regs* r) {
    void* state = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->edi));

    const uint8_t gate = GP4_FIELD(uint8_t, state, 0x9c);
    if ((gate & 0x10u) != 0u) {
        r->eax = 0u;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(gate & 0x10u);
        return;
    }

    const uint32_t base = GP4_FIELD(uint16_t, record, 0x16);
    const uint32_t diff = (GP4_FIELD(uint16_t, record, 0x176) - base) & 0xffffu;
    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(diff))
        * static_cast<int32_t>(GP4_FIELD(int16_t, state, 0x66));
    const uint32_t low = static_cast<uint32_t>(product) & 0xffffu;
    const uint32_t high = (static_cast<uint32_t>(product) >> 16) & 0xffffu;

    // shl ax,1 / rcl dx,1 twice: dx takes the top two bits of ax
    const uint32_t shifted = ((high << 2) | (low >> 14)) & 0xffffu;
    const uint32_t sum = (shifted + base) & 0xffffu;

    r->eax = sum;
    r->edx = (r->edx & 0xffff0000u) | shifted;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(shifted, base);
}
GP4_IMPL(0x004030fe, FUN_004030fe, "regs(esi:ptr[0xa4]:bytes, edi:ptr[0x17e]:bytes, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of)")
