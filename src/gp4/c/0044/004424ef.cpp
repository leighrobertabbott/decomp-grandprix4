#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
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
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Schedules the next time-limited event: [0x0066d83c] = clock [0x0066d814] + delay, [0x0066d898] = 0. The
// delay is a day (86400000 ms) when the byte 0x007c2e6c is set, when only one driver counts or when the
// estimate degenerates; otherwise it is estimated from a lap fraction (0x43b862, or the word 0x007c2e96 << 8
// when the byte 0x0062ad78 is set) scaled by [0x0062d238] >> 8, divided by the remaining-driver figure
// (word 0x007ad89c clamped to >= 1 times the active-driver count [0x0066d89a] minus the sum [0x0066d894],
// both from 0x442f34) and shifted by 10, minus 0x6605 (plus 0x4268 when the byte 0x0062ad78 is set). With
// the byte set the word 0x007ad89c is decremented until the estimate reaches 0x798d or the word is 1.
// pushal / popal restore every register; the flags are those of the final add.
// FUNCTION: GP4 0x004424ef
void __cdecl FUN_004424ef(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00442f34, c);

    uint32_t delay = 0x05265c00u;

    if (GP4_GLOBAL(uint8_t, 0x007c2e6c) == 0u) {
        uint32_t estimate;
        uint32_t divisor;
        for (;;) {
            uint32_t lap = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96)) << 8;
            if (GP4_GLOBAL(uint8_t, 0x0062ad78) == 0u) {
                c = *r;
                c.st_in = 0;
                c.st_out = 0;
                gp4::call_regs(0x0043b862, c);
                lap = c.eax;
            }

            uint32_t scaled = ((GP4_GLOBAL(uint32_t, 0x0062d238) >> 8) * lap) >> 10;
            if (static_cast<int32_t>(scaled) > 0x7fff) {
                scaled = 0x7fffu;
            }

            int16_t laps = static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x007ad89c));
            if (laps < 1) {
                laps = 1;
            }
            int16_t remaining = static_cast<int16_t>(static_cast<uint16_t>(
                static_cast<uint16_t>(laps) * GP4_GLOBAL(uint16_t, 0x0066d89a)) - GP4_GLOBAL(uint16_t, 0x0066d894));
            if (remaining < 1) {
                remaining = 1;
            }
            divisor = static_cast<uint16_t>(remaining);

            estimate = (scaled / divisor) << 10;
            if (static_cast<int32_t>(estimate) >= 0x798d) {
                break;
            }
            if (GP4_GLOBAL(uint8_t, 0x0062ad78) == 0u || GP4_GLOBAL(uint16_t, 0x007ad89c) == 1u) {
                estimate = 0x798du;
                break;
            }
            GP4_GLOBAL(uint16_t, 0x007ad89c) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x007ad89c) - 1u);
        }

        estimate -= 0x6605u;
        if (GP4_GLOBAL(uint8_t, 0x0062ad78) != 0u) {
            estimate += 0x4268u;
        }
        if (divisor != 1u && static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x0066d89a)) > 1) {
            delay = estimate;
        }
    }

    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_GLOBAL(uint32_t, 0x0066d83c) = clock;
    GP4_GLOBAL(uint32_t, 0x0066d83c) = clock + delay;
    GP4_GLOBAL(uint16_t, 0x0066d898) = 0;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(clock, delay);
}
GP4_IMPL(0x004424ef, FUN_004424ef, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+cars_0066fef0+lap_0043b482+carno_0066ff6c+state_0040d8a5")
