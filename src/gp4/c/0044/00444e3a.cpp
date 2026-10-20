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

// Flags of an OR / AND / TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags32(uint32_t result) {
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// When the car pointer 0x0066d820 is set, appends the two-byte record {8, [0x007a90fc]} to the opcode stream
// whose write cursor is the pointer 0x0062d1a0 and advances the cursor by 2. pushal / popal restore every
// register; the flags are those of the `or esi, esi` test (no car) or of the final add.
// FUNCTION: GP4 0x00444e3a
void __cdecl FUN_00444e3a(gp4::Regs* r) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x0066d820);
    uint32_t flags = logicFlags32(car);

    if (car != 0u) {
        uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));
        cursor[0] = 8u;
        cursor[1] = GP4_GLOBAL(uint8_t, 0x007a90fc);

        const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
        GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 2u;
        flags = addFlags32(before, 2u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444e3a, FUN_00444e3a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+stream_0062d1a0")
