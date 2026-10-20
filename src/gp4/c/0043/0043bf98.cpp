#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043bf98
// Builds a histogram of the track's surface samples. For every eighth segment record of the
// 0x160-byte array (0x8943a0, step 0xb00, up to the lap end 0x00866ecc) the sample selected by word
// +0x7a (10-byte entries at 0x00675a82) is scored by 0x0043bee0: its byte +1 is summed into
// 0x00622780, its edx into 0x00622790 and the number of samples counted in 0x0062277c. The score
// (capped at 0xb0) falls into one of 12 bins of 16: the bin 0x0063b8b0[score >> 4] gains
// 16 - (score & 15) and, when the remainder is not zero, the next bin 0x0063b8b4[score >> 4] gains the
// remainder. Bins are then scaled to Q14 of the sample count (x 16), and both sums become averages.
// pushal/popal restore every register; popal does not restore the flags, which are those of the final
// `xor edx, edx` ahead of the last `idiv` (the oracle's idiv keeps them): ZF and PF.
void __cdecl FUN_0043bf98(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (uint32_t bin = 0u; bin != 12u; ++bin) {
        GP4_ARRAY(uint32_t, 0x0063b8b0)[bin] = 0u;
    }
    uint32_t samples = 0u;
    GP4_GLOBAL(uint32_t, 0x00622780) = 0u;
    GP4_GLOBAL(uint32_t, 0x00622790) = 0u;
    GP4_GLOBAL(uint32_t, 0x0062277c) = 0u;

    uint32_t record = 0x008943a0u;
    do {
        const uint32_t sample = static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(record + 0x7au))) * 10u + 0x00675a82u;
        GP4_GLOBAL(uint32_t, 0x00622780) += *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(sample + 1u));
        GP4_GLOBAL(uint32_t, 0x0062277c) += 1u;

        c.ebx = sample;
        gp4::call_regs(0x0043bee0, c);
        GP4_GLOBAL(uint32_t, 0x00622790) += c.edx;

        int32_t score = static_cast<int32_t>(c.eax);
        if (score > 0xb0) {
            score = 0xb0;
        }
        const uint32_t bin = static_cast<uint32_t>(score) >> 4;
        const uint32_t remainder = static_cast<uint32_t>(score) & 0xfu;
        if (remainder != 0u) {
            GP4_ARRAY(uint32_t, 0x0063b8b4)[bin] += remainder;
        }
        GP4_ARRAY(uint32_t, 0x0063b8b0)[bin] += 0x10u - remainder;

        samples += 0x10u;
        record += 0xb00u;
    } while (record < GP4_GLOBAL(uint32_t, 0x00866ecc));

    int32_t rem;
    for (uint32_t bin = 0u; bin != 12u; ++bin) {
        const uint32_t scaled = GP4_ARRAY(uint32_t, 0x0063b8b0)[bin] << 14;
        GP4_ARRAY(uint32_t, 0x0063b8b0)[bin] = static_cast<uint32_t>(
            gp4::idiv64(static_cast<int64_t>(scaled), static_cast<int32_t>(samples), &rem));
    }
    GP4_GLOBAL(uint32_t, 0x00622780) = static_cast<uint32_t>(gp4::idiv64(
        static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x00622780)), GP4_GLOBAL(int32_t, 0x0062277c), &rem));
    GP4_GLOBAL(uint32_t, 0x00622790) = static_cast<uint32_t>(gp4::idiv64(
        static_cast<int64_t>(GP4_GLOBAL(uint32_t, 0x00622790)), GP4_GLOBAL(int32_t, 0x0062277c), &rem));

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bf98, FUN_0043bf98, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0040c0ae+lim_00937dd0")
