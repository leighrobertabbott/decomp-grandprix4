#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the brightness remap tables (pushal/popal keep every register):
//  - 0x00922300: 16 rows of 32 bytes, entry (row, col) = min(31, (col * ((0x1a80 + 0x280 * row) >> 8)) >> 6);
//  - 0x009221c0: ten copies of row 0 (32 bytes each, 0x140 bytes up to 0x00922300);
//  - 0x00922504: 16 rows of 32 bytes, column col holds ((step * k) >> 12) + col for k = 1..16 down the
//    column (rows 0x20 apart) where step = (((byte [0x00922500] >> 3) - col) << 12) / 16.
// The flags are those of the final `cmp ecx, 0x20` (equal).
// FUNCTION: GP4 0x0051bc7b
void __cdecl FUN_0051bc7b(gp4::Regs* r) {
    uint32_t scale = 0x1a80u;
    for (uint32_t row = 0u; row != 0x10u; ++row) {
        for (int32_t col = 0x1f; col >= 0; --col) {
            int32_t level = static_cast<int32_t>(static_cast<uint32_t>(col) * (scale >> 8)) >> 6;
            if (level > 0x1f) {
                level = 0x1f;
            }
            GP4_ARRAY(uint8_t, 0x00922300)[row * 0x20u + static_cast<uint32_t>(col)] = static_cast<uint8_t>(level);
        }
        scale += 0x280u;
    }

    uint32_t* dst = GP4_ARRAY(uint32_t, 0x009221c0);
    for (uint32_t copy = 0u; copy != 10u; ++copy) {
        const uint32_t* src = GP4_ARRAY(uint32_t, 0x00922300);
        for (uint32_t i = 0u; i != 8u; ++i) {
            *dst++ = *src++;
        }
    }

    for (uint32_t col = 0u; col < 0x20u; ++col) {
        uint8_t* out = GP4_ARRAY(uint8_t, 0x00922504) + col;
        const int32_t base = static_cast<int32_t>((GP4_GLOBAL(uint32_t, 0x00922500) & 0xffu) >> 3);
        const int32_t step = ((base - static_cast<int32_t>(col)) * 0x1000) / 0x10;
        uint32_t acc = static_cast<uint32_t>(step);
        for (uint32_t k = 0u; k < 0x10u; ++k) {
            *out = static_cast<uint8_t>((acc >> 12) + col);
            out += 0x20;
            acc += static_cast<uint32_t>(step);
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x20u, 0x20u));
}
GP4_IMPL(0x0051bc7b, FUN_0051bc7b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
