#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "shl reg32, count" for count in 2..31 under the pinned oracle model:
// CF = last bit shifted out, AF = 0, OF = bit31((value << (count-1)) ^ result).
static inline uint32_t shlFlags(uint32_t value, uint32_t count) {
    const uint32_t before = value << (count - 1u);
    const uint32_t result = value << count;
    return ((before >> 31) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((before ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
}

// Rotates the coordinate pair (eax, edx) (>> 8, then >> 1 each) into sum +- scaled difference
// (scale DAT_00629db8 in Q14), returns both results << 8 and tracks the largest |new - old|
// in DAT_00629e08. The original compares the first deviation (edx) a second time, so the
// second deviation (eax) is stored whenever the first one was stored.
// FUNCTION: GP4 0x00436df3
void __cdecl FUN_00436df3(gp4::Regs* r) {
    const int32_t first8 = (int32_t)r->eax >> 8;
    const int32_t second8 = (int32_t)r->edx >> 8;
    const int32_t first = first8 >> 1;
    const int32_t second = second8 >> 1;

    const uint32_t sum = (uint32_t)first + (uint32_t)second;
    const uint32_t difference = (uint32_t)first - (uint32_t)second;
    const int64_t product = (int64_t)(int32_t)difference * (int64_t)GP4_GLOBAL(int32_t, 0x00629db8);
    const uint32_t delta = (uint32_t)(uint64_t)(product >> 14);

    const uint32_t low = sum - delta;
    const uint32_t high = sum + delta;

    int32_t deviationHigh = (int32_t)(high - (uint32_t)second8);
    if (deviationHigh < 0) {
        deviationHigh = (int32_t)(0u - (uint32_t)deviationHigh);
    }
    if (!(deviationHigh < GP4_GLOBAL(int32_t, 0x00629e08))) {
        GP4_GLOBAL(int32_t, 0x00629e08) = deviationHigh;
    }

    int32_t deviationLow = (int32_t)(low - (uint32_t)first8);
    if (deviationLow < 0) {
        deviationLow = (int32_t)(0u - (uint32_t)deviationLow);
    }
    if (!(deviationHigh < GP4_GLOBAL(int32_t, 0x00629e08))) {
        GP4_GLOBAL(int32_t, 0x00629e08) = deviationLow;
    }

    r->eax = low << 8;
    r->edx = high << 8;
    r->ecx = (uint32_t)first8;
    r->eflags = (r->eflags & ~0x8d5u) | shlFlags(high, 8u);
}
GP4_IMPL(0x00436df3, FUN_00436df3, "regs(eax:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
