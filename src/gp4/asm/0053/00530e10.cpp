#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper over a device record (ebx, 0x3c-byte records at 0x827160). The record holds a bitmask byte
// +4, two column indices +0xc and +0x10 (0..6) and seven setting dwords +0x28..+0x40. Table 0x008273e3 holds
// seven dwords; the span between the two selected columns gives a slope
// ((setting[+0x10] - setting[+0xc]) << 16) / (table[+0x10] - table[+0xc]) (saturated to +-0x7fffffff when the
// divisor is under twice the quotient's high part), stored in 0x0082745d together with the intercept
// (0x00827461) and the column-6 scale (0x00827465 = slope * 0x957d >> 16). Each masked column whose setting is
// still zero gets table[col] * slope >> 16 + intercept (at least 1; column 6 adds a (table[6] - table[5]) term
// of at least 0x200). Columns outside the [+0xc, +0x10] range then take the larger of that value and the
// speed-dependent default from 0x00531445. popal restores every register; the flags are those of the final
// `cmp ecx, 7` (or of the equal compare on the early exit): a zero-difference compare either way.
// FUNCTION: GP4 0x00530e10
void __cdecl FUN_00530e10(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t from = *reinterpret_cast<uint32_t*>(record + 0x0c);
    const uint32_t to = *reinterpret_cast<uint32_t*>(record + 0x10);
    uint32_t* settings = reinterpret_cast<uint32_t*>(record + 0x28);
    const uint32_t* table = GP4_ARRAY(const uint32_t, 0x008273e3);

    if (from != to) {
        const uint32_t span = table[to] - table[from];
        const int32_t delta = static_cast<int32_t>(settings[to] - settings[from]);
        const int32_t high = delta >> 16;
        const uint32_t magnitude = static_cast<uint32_t>(high < 0 ? -high : high);

        int32_t slope;
        if ((span >> 1) > magnitude) {
            if (span == 0u) {
                slope = 0;
            } else {
                int32_t remainder;
                slope = gp4::idiv64(static_cast<int64_t>(delta) << 16, static_cast<int32_t>(span), &remainder);
            }
        } else if (high < 0) {
            slope = static_cast<int32_t>(0x80000001u);
        } else {
            slope = 0x7fffffff;
        }
        GP4_GLOBAL(int32_t, 0x0082745d) = slope;

        const int64_t first = static_cast<int64_t>(slope) * static_cast<int32_t>(table[from]);
        const uint32_t intercept = settings[from] - static_cast<uint32_t>(static_cast<uint64_t>(first) >> 16);
        GP4_GLOBAL(uint32_t, 0x00827461) = intercept;

        const int64_t scaled_slope = static_cast<int64_t>(0x957d) * slope;
        const uint32_t column6_scale = static_cast<uint32_t>(static_cast<uint64_t>(scaled_slope) >> 16);
        GP4_GLOBAL(uint32_t, 0x00827465) = column6_scale;

        const uint8_t mask = record[4];
        gp4::Regs c = *r;
        for (uint32_t column = 0u; column != 7u; ++column) {
            if ((mask & (1u << column)) == 0u || settings[column] != 0u) {
                continue;
            }
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(table[column])) * slope;
            const uint32_t product_high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16) + intercept);
            if (value < 1) {
                value = 1;
            }
            if (column == 6u) {
                const uint32_t step = (table[6] - table[5]) * column6_scale;
                int32_t extra = static_cast<int32_t>((step >> 16) | (product_high << 16));
                if (extra < 0x200) {
                    extra = 0x200;
                }
                value = static_cast<int32_t>(static_cast<uint32_t>(extra) + static_cast<uint32_t>(value));
            }
            uint32_t result = static_cast<uint32_t>(value);
            if (column < from || column > to) {
                c.eax = result;
                c.ecx = column;
                gp4::call_regs(0x00531445, c);
                if (result < c.edx) {
                    result = c.edx;
                }
            }
            settings[column] = result;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(7u, 7u));
}
GP4_IMPL(0x00530e10, FUN_00530e10, "regs(ebx:ptr[0x44]:layout=dev_00530e10) -> (cf, pf, af, zf, sf, of); globals=layout=rate_00530e10")
