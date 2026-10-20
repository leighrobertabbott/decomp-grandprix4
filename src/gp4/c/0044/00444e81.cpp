#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Appends the five-byte record {9, [0x008914e0] & 0x60, flag byte from 0x00445218, [0x008914e2],
// [0x008914e2] & 0xf} to the opcode stream whose write cursor is the pointer at 0x0062d1a0 and advances the
// cursor by 5. pushal / popal restore every register; only the flags of the final
// "add dword [cursor], 5" survive.
// FUNCTION: GP4 0x00444e81
void __cdecl FUN_00444e81(gp4::Regs* r) {
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));
    cursor[0] = 9u;
    const uint8_t mode = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008914e0) & 0x60u);
    cursor[1] = mode;

    gp4::Regs call = *r;
    call.eax = (r->eax & 0xffffff00u) | mode;
    gp4::call_regs(0x00445218, call);
    cursor[2] = static_cast<uint8_t>(call.eax);

    const uint8_t level = GP4_GLOBAL(uint8_t, 0x008914e2);
    cursor[3] = level;
    cursor[4] = static_cast<uint8_t>(level & 0xfu);

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 5u;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 5u);
}
GP4_IMPL(0x00444e81, FUN_00444e81, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0+flags_00445218")
