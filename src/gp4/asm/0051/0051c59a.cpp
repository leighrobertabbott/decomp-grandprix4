#include <gp4/gp4.h>
#include <gp4/flags.h>

// Remaps every texel of the eight mip levels of the texture entry in eax in place. Entry: +0
// pointer to eight level buffers, +4 width (u16), +6 height (u16) of level 0. Level L is H rows of
// W bytes with a row stride of 0x100; each texel b becomes lut[(L << 8) | map[b]] (map = 0x00948858,
// lut = 0x00947e10). W and H halve after every level with a floor of 1. The count loops are
// `loop` / `dec ebp; jne`, so a zero count runs 2^32 times (do-while on unsigned counters).
// pushal/popal restore every register; the flags are those of the final `cmp ebx, 8` (equal).
// FUNCTION: GP4 0x0051c59a
void __cdecl FUN_0051c59a(gp4::Regs* r) {
    const uint32_t entry = r->eax;
    uint32_t width = GP4_FIELD(uint16_t, entry, 4);
    uint32_t height = GP4_FIELD(uint16_t, entry, 6);
    const uint32_t levels = GP4_FIELD(uint32_t, entry, 0);

    for (uint32_t level = 0; level != 8u; ++level) {
        uint8_t* row = GP4_FIELD(uint8_t*, levels, level * 4u);
        uint32_t rows = height;
        do {
            uint8_t* texel = row;
            uint32_t count = width;
            do {
                const uint32_t mapped = GP4_ARRAY(uint8_t, 0x00948858)[*texel];
                *texel = GP4_ARRAY(uint8_t, 0x00947e10)[(level << 8) | mapped];
                ++texel;
                --count;
            } while (count != 0u);
            row += 0x100;
            --rows;
        } while (rows != 0u);

        width >>= 1;
        if (static_cast<int32_t>(width) < 1) {
            width = 1u;
        }
        height >>= 1;
        if (static_cast<int32_t>(height) < 1) {
            height = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(8u, 8u));
}
GP4_IMPL(0x0051c59a, FUN_0051c59a, "regs(eax:ptr[0x20]:layout=tex_0051c59a) -> (cf, pf, af, zf, sf, of); globals=layout=lut_0051c59a")
