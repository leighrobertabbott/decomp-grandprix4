#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal register routine called by the palette builder 0x00527592. For each of the 32
// colour slots [0x00948b80 + row] (row) and each slot [0x00948b80 + col] (col) it stores one byte
// at 0x00948c00 + row * 0x100 + col:
//  - row colour 0x15: the column number itself;
//  - otherwise delta = clamp(colour[col] - 0x15, -15, 0) and the remap byte [0x00948a58 + i], with
//    i = [0x00923104 + (delta + 15) * 0x100 + colour[row]] when colour[row] >= 0x28, else
//    i = clamp(colour[row] + delta, 0, 0x27).
// The 32 bytes of each row are then expanded into 7 further 32-byte groups (0x00948c20 + row *
// 0x100 ...): group k = (group k-1 & 0x1f1f1f1f) | k * 0x20202020, dword by dword.
// popal keeps the flags of the final `cmp ebx, 0x20`.
// FUNCTION: GP4 0x00527787
void __cdecl FUN_00527787(gp4::Regs* r) {
    const uint8_t* colours = GP4_ARRAY(const uint8_t, 0x00948b80);
    const uint8_t* shades = GP4_ARRAY(const uint8_t, 0x00923104);
    const uint8_t* remap = GP4_ARRAY(const uint8_t, 0x00948a58);

    for (uint32_t row = 0; row < 0x20; ++row) {
        uint8_t* out = GP4_ARRAY(uint8_t, 0x00948c00) + row * 0x100;
        for (uint32_t col = 0; col < 0x20; ++col) {
            uint8_t value;
            if (colours[row] == 0x15) {
                value = (uint8_t)col;
            } else {
                int32_t delta = (int32_t)colours[col] - 0x15;
                if (delta > 0) {
                    delta = 0;
                }
                if (delta < -15) {
                    delta = -15;
                }
                const uint32_t base = colours[row];
                uint32_t index;
                if (base >= 0x28) {
                    index = shades[((uint32_t)(delta + 15) << 8) | base];
                } else {
                    int32_t level = delta + (int32_t)base;
                    if (level < 0) {
                        level = 0;
                    }
                    if (level > 0x27) {
                        level = 0x27;
                    }
                    index = (uint32_t)level;
                }
                value = remap[index];
            }
            out[col] = value;
        }

        const uint32_t* src = reinterpret_cast<const uint32_t*>(out);
        uint32_t* dst = reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x00948c20) + row * 0x100);
        uint32_t fill = 0x20202020u;
        for (int group = 1; group < 8; ++group) {
            for (int i = 0; i < 8; ++i) {
                *dst++ = (*src++ & 0x1f1f1f1fu) | fill;
            }
            fill += 0x20202020u;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x20u, 0x20u));
}
GP4_IMPL(0x00527787, FUN_00527787, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pal_00527787")
