#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Sets the byte flag 0x00625a4b to 0xff when eax is non-zero, otherwise to 0. The flags are those of
// `test eax, -1` (CF = OF = AF = 0).
// FUNCTION: GP4 0x00440ddf
void __cdecl FUN_00440ddf(gp4::Regs* r) {
    const uint32_t value = r->eax;
    GP4_GLOBAL(uint8_t, 0x00625a4b) = (value != 0u) ? 0xffu : 0u;

    const uint32_t flags = parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00440ddf, FUN_00440ddf, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
