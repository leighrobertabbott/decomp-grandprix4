#include <gp4/gp4.h>

static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00403800
// edi is a position in a wrapping array whose wrap threshold is DAT_00866f10 and wrap length
// DAT_00866ec4; the block at DAT_00866ed4 - 0x2940 holds the other bound at +0x44. eax is
// DAT_00852c44 + 0x50 when edi lies before that bound (or before the block itself when edi is at
// or past DAT_00891500), else DAT_00852c48 + 0x50. Flags are those of the final add; edi is preserved.
void __cdecl FUN_00403800(gp4::Regs* r) {
    uint32_t position = r->edi;
    const uint32_t block = GP4_GLOBAL(uint32_t, 0x00866ed4) - 0x2940u;

    bool before;
    if (position < GP4_GLOBAL(uint32_t, 0x00891500)) {
        if (position < GP4_GLOBAL(uint32_t, 0x00866f10)) {
            position += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
        uint32_t bound = GP4_FIELD(uint32_t, (uint8_t*)(uintptr_t)block, 0x44);
        if (bound < GP4_GLOBAL(uint32_t, 0x00866f10)) {
            bound += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
        before = position < bound;
    } else {
        before = position < block;
    }

    const uint32_t value = before ? GP4_GLOBAL(uint32_t, 0x00852c44) : GP4_GLOBAL(uint32_t, 0x00852c48);
    r->eax = value + 0x50u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(value, 0x50u);
}
GP4_IMPL(0x00403800, FUN_00403800, "regs(edi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00403800")
