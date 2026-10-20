#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit ADD left, right.
static inline uint32_t addFlags8(uint8_t left, uint8_t right) {
    const uint32_t sum = static_cast<uint32_t>(left) + static_cast<uint32_t>(right);
    const uint8_t result = static_cast<uint8_t>(sum);
    return (sum > 0xffu ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Folds one 0x41c-byte block (the dwords of the record at 0x0066fef0 + [0x0066d88e], masked with the
// dword table at 0x0062ad7c) into a rotating sum (rol 1 every fifth dword), then reduces the sum to a
// byte checksum stored at 0x008914e2. All registers are saved/restored; the flags left are those of
// the final byte add.
// FUNCTION: GP4 0x0044552d
void __cdecl FUN_0044552d(gp4::Regs* r) {
    const uint32_t* block = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(0x0066fef0u + static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0066d88e))));
    uint32_t sum = 0;
    uint32_t countdown = 5;
    for (uint32_t i = 0; i < 0x107u; ++i) {
        sum += block[i] & GP4_ARRAY(uint32_t, 0x0062ad7c)[i];
        countdown -= 1;
        if (countdown == 0) {
            sum = (sum << 1) | (sum >> 31);
            countdown = 5;
        }
    }

    const uint16_t folded = static_cast<uint16_t>(static_cast<uint16_t>(sum >> 16) + static_cast<uint16_t>(sum));
    const uint8_t low = static_cast<uint8_t>(folded);
    const uint8_t high = static_cast<uint8_t>(folded >> 8);
    GP4_GLOBAL(uint8_t, 0x008914e2) = static_cast<uint8_t>(low + high);

    r->eflags = (r->eflags & ~0x8d5u) | addFlags8(low, high);
}
GP4_IMPL(0x0044552d, FUN_0044552d, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+cur_0066d88e")
