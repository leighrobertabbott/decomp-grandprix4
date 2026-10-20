#include <gp4/gp4.h>

// Flags of the 8-bit `cmp value, 0`: CF = OF = AF = 0, the rest from the value itself.
static inline uint32_t cmpZeroFlags8(uint32_t value) {
    value &= 0xffu;
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 0x04u : 0u) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Number of cars the race display can show: a fixed-point estimate from the circuit table [0x007ad89e], the
// scale [0x0062d238] and the window [0x007c2ea0] (at least 5), divided out as a 64/32 DIV, plus one, capped to
// 0x10 (1 when the mode byte [0x007c2ea9] is zero). The count goes into the word [0x0066d8f0] and the pace
// factor (3: the retry loop that lowers it would need it above 4) into [0x00679b50]. pushal / popal restore
// every register; the flags are those of `cmp byte [0x007c2ea9], 0`.
// FUNCTION: GP4 0x00442fa3
void __cdecl FUN_00442fa3(gp4::Regs* r) {
    uint32_t pace = 3u;
    uint32_t count;

    for (;;) {
        const uint32_t circuit = GP4_GLOBAL(uint16_t, 0x007ad89e);
        const int32_t scaled = static_cast<int32_t>((4u - circuit) * 0x147u + 0x4000u);
        const int64_t product = static_cast<int64_t>(scaled) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062d238));
        const uint32_t base = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        const uint32_t sum = base * pace + 60000u;
        const uint32_t estimate = (sum * 11u) >> 8;
        const uint64_t wide = static_cast<uint64_t>(estimate) * 0x800u;
        const uint32_t low = static_cast<uint32_t>(wide) >> 8;
        const uint32_t high = static_cast<uint32_t>(wide >> 32);

        uint32_t window = GP4_GLOBAL(uint16_t, 0x007c2ea0);
        if (static_cast<int32_t>(window) < 5) {
            window = 5u;
        }
        uint32_t remainder;
        count = gp4::div64((static_cast<uint64_t>(high) << 32) | low, window * 0xeau, &remainder) + 1u;

        if (static_cast<int32_t>(count) <= 0x10) {
            break;
        }
        if (static_cast<int32_t>(pace) <= 4) {
            count = 0x10u;
            break;
        }
        --pace;
    }

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode == 0u) {
        count = 1u;
    }
    GP4_GLOBAL(uint16_t, 0x0066d8f0) = static_cast<uint16_t>(count);
    GP4_GLOBAL(uint32_t, 0x00679b50) = pace;

    r->eflags = (r->eflags & ~0x8d5u) | cmpZeroFlags8(mode);
}
GP4_IMPL(0x00442fa3, FUN_00442fa3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004320f2")
