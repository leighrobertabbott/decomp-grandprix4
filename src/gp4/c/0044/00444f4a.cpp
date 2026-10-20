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

// Appends a two-byte record (opcode 0x0c, then the byte at 0x0066d8b4) to the opcode stream whose write cursor
// is the pointer 0x0062d1a0 and advances the cursor by 2. push/pop restore eax and ebx; only the flags of the
// final "add dword [cursor], 2" survive.
// FUNCTION: GP4 0x00444f4a
void __cdecl FUN_00444f4a(gp4::Regs* r) {
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));
    cursor[0] = 0x0cu;
    cursor[1] = GP4_GLOBAL(uint8_t, 0x0066d8b4);

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 2u;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 2u);
}
GP4_IMPL(0x00444f4a, FUN_00444f4a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0")
