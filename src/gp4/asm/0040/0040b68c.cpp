#include <gp4/gp4.h>

// Q14 sine/cosine style lookup used by the loop: |angle| >> 2 (even byte offset) into the word table
// at 0x007c5c5c.
static inline int32_t tableValue(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    const uint32_t offset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return static_cast<int32_t>(GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), offset));
}

// (value * DAT_0062250c) >> 14 (imul + shrd 14) followed by sar 2.
static inline int32_t scaledStep(int32_t value) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062250c));
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14)) >> 2;
}

// FUNCTION: GP4 0x0040b68c
// pushal / popal routine: every register is preserved, only memory and the flags of its last cmp
// remain. Builds a histogram of the track surface map (0x0040c43b, values 0..5) sampled along
// direction rays: for each angle from -DAT_00622516 up to +DAT_00622516 in steps of DAT_00622518
// and each of the 5 distance groups, 112 steps of (esi, edi) = the table cosine / sine scaled by
// DAT_0062250c are walked from the origin. The counts per (group, value) go to 0x0062accc (6 dwords
// per group), the per-group totals to 0x0062acb8, and the percentages (count * 100 / total) to the
// bytes 0x007a8eb0.
void __cdecl FUN_0040b68c(gp4::Regs* r) {
    uint32_t* cells = GP4_ARRAY(uint32_t, 0x0062accc);
    uint32_t* totals = GP4_ARRAY(uint32_t, 0x0062acb8);
    for (uint32_t i = 0; i < 0x1eu; ++i) {
        cells[i] = 0;
    }
    for (uint32_t i = 0; i < 5u; ++i) {
        totals[i] = 0;
    }

    uint16_t angle = static_cast<uint16_t>(0u - GP4_GLOBAL(uint16_t, 0x00622516));
    totals[0] = 0;
    do {
        const uint16_t sum = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00622514) + angle);
        const int32_t stepX = scaledStep(tableValue(static_cast<uint16_t>(0x4000u - sum)));
        const int32_t stepY = scaledStep(tableValue(sum));

        uint32_t x = 0;
        uint32_t y = 0;
        GP4_GLOBAL(uint32_t, 0x0062acb4) = 0;
        do {
            for (uint32_t step = 0x708u >> 4; step != 0u; --step) {
                x -= static_cast<uint32_t>(stepX);
                y -= static_cast<uint32_t>(stepY);
                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                call.eax = x;
                call.ebx = x;
                call.edx = y;
                gp4::call_regs(0x0040c43b, call);
                const uint32_t group = GP4_GLOBAL(uint32_t, 0x0062acb4);
                cells[call.eax + group * 6u] += 1u;
                totals[GP4_GLOBAL(uint32_t, 0x0062acb4)] += 1u;
            }
            GP4_GLOBAL(uint32_t, 0x0062acb4) += 1u;
        } while (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062acb4)) < 5);

        angle = static_cast<uint16_t>(angle + GP4_GLOBAL(uint16_t, 0x00622518));
    } while (static_cast<int16_t>(angle) <= GP4_GLOBAL(int16_t, 0x00622516));

    for (uint32_t row = 0; row < 5u; ++row) {
        for (uint32_t column = 0; column < 6u; ++column) {
            const uint32_t index = row * 6u + column;
            const int64_t dividend = static_cast<int64_t>(static_cast<int32_t>(cells[index])) * 100;
            int32_t divisor = static_cast<int32_t>(totals[row]);
            if (divisor == 0) {
                divisor = 1;
            }
            int32_t remainder;
            GP4_ARRAY(uint8_t, 0x007a8eb0)[index] = static_cast<uint8_t>(gp4::idiv64(dividend, divisor, &remainder));
        }
    }

    // The final `cmp ebx, 5` with ebx == 5 (popal does not restore the flags): ZF and PF.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040b68c, FUN_0040b68c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=map_0040c43b+state_0040bce6")
