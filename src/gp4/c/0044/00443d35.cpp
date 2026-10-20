#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Sets the flag byte 0x0066d88c, draws a random byte from the generator 0x00401462 and stores 1000 + 16 * byte in
// 0x00679b1c, then sets 0x0066d844 to 1500. eax ends as the stored value; the flags are those of the final add.
// FUNCTION: GP4 0x00443d35
void __cdecl FUN_00443d35(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0066d88c) = 1u;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401462, call);

    const uint32_t scaled = (call.eax & 0xffu) * 0x10u;
    const uint32_t value = scaled + 0x3e8u;
    GP4_GLOBAL(uint32_t, 0x00679b1c) = value;
    GP4_GLOBAL(uint32_t, 0x0066d844) = 0x5dcu;

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(scaled, 0x3e8u);
}
GP4_IMPL(0x00443d35, FUN_00443d35, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
