#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040b081
// Sets bit 2 of byte +0x12 in six consecutive 0x160-byte records starting 0x11300 bytes
// before [0x00866ecc]. ecx/edi are preserved; the flags are those of the last `add edi, 0x160`.
void __cdecl FUN_0040b081(gp4::Regs* r) {
    uint32_t address = GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x11300u;
    uint32_t lastLeft = 0u;
    for (int remaining = 6; remaining != 0; --remaining) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
        GP4_FIELD(uint8_t, record, 0x12) |= 2u;
        lastLeft = address;
        address += 0x160u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(lastLeft, 0x160u);
}
GP4_IMPL(0x0040b081, FUN_0040b081, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=records_008943a0")
