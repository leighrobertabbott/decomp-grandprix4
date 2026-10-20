#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Appends the opcode byte 0x0b to the byte stream whose write cursor is the pointer 0x0062d1a0 and advances
// the cursor. pushal / popal restore every register; only the flags of the final "add dword [cursor], 1" survive.
// FUNCTION: GP4 0x00444f1e
void __cdecl FUN_00444f1e(gp4::Regs* r) {
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));
    *cursor = 0x0bu;

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    const uint32_t result = before + 1u;
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = result;

    const uint32_t flags = (result < 1u ? 1u : 0u)
        | parityFlag(result)
        | (((before ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(before ^ 1u) & (before ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444f1e, FUN_00444f1e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0")
