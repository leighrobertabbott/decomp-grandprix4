#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the 64x64 colour lookup of the palette builder 0x527592. Points the nearest-colour search
// table [0x96a3d4] at the 6-byte {word, word, word} palette 0x7c4d5c, lets 0x51b6d4 flag the 32 colour
// bytes of 0x948b80 in the map 0x954060, and with [0x95405e] = 0xff (search only flagged entries):
// - 0x94ac00[i] = (hi byte of word 0 << 12) | hi byte of word 2 of palette entry 0x948b80[i], i < 32;
// - 0x94ac80 gets seven copies of those 32 dwords with the top byte replaced by 1..7;
// - for c = 0..0xfff, the point (x = y = ((c >> 6) & 0x3f) << 8, z = (c & 0x3f) << 8) goes to
//   0x933a14/16/18, 0x51b645 returns the nearest flagged entry in al and 0x94b000[c] = 0x948858[al].
// [0x95405e] is cleared at the end. pushal/popal keep every register; the flags are those of the
// final `cmp ecx, 0x1000` (equal).
// FUNCTION: GP4 0x005278e5
void __cdecl FUN_005278e5(gp4::Regs* r) {
    gp4::Regs c = *r;
    GP4_GLOBAL(uint32_t, 0x0096a3d4) = 0x007c4d5cu;
    gp4::call_regs(0x0051b6d4, c);
    GP4_GLOBAL(uint8_t, 0x0095405e) = 0xffu;

    const uint8_t* colours = GP4_ARRAY(const uint8_t, 0x00948b80);
    uint32_t* packed = GP4_ARRAY(uint32_t, 0x0094ac00);
    for (uint32_t i = 0u; i != 0x20u; ++i) {
        const uint16_t* entry = reinterpret_cast<const uint16_t*>(0x007c4d5cu + colours[i] * 6u);
        packed[i] = (static_cast<uint32_t>(entry[0] >> 8) << 12) | static_cast<uint32_t>(entry[2] >> 8);
    }

    uint32_t* out = GP4_ARRAY(uint32_t, 0x0094ac80);
    uint32_t level = 0x01000000u;
    for (uint32_t pass = 7u; pass != 0u; --pass) {
        for (uint32_t i = 0u; i != 0x20u; ++i) {
            *out++ = (packed[i] & 0x00ffffffu) | level;
        }
        level += 0x01000000u;
    }

    // Registers as the original leaves them at the search calls.
    c.edx = level;
    c.ebp = 0u;
    c.esi = 0x0094ac80u;
    c.edi = reinterpret_cast<uint32_t>(out);
    uint32_t cell = 0u;
    do {
        GP4_GLOBAL(uint16_t, 0x00933a18) = static_cast<uint16_t>((cell & 0x3fu) << 8);
        const uint32_t xy = ((cell >> 6) & 0x3fu) << 8;
        GP4_GLOBAL(uint16_t, 0x00933a14) = static_cast<uint16_t>(xy);
        GP4_GLOBAL(uint16_t, 0x00933a16) = static_cast<uint16_t>(xy);
        c.eax = xy;
        c.ecx = cell;
        gp4::call_regs(0x0051b645, c);
        cell = c.ecx;
        GP4_ARRAY(uint8_t, 0x0094b000)[cell] = GP4_ARRAY(const uint8_t, 0x00948858)[c.eax & 0xffu];
        ++cell;
    } while (cell != 0x1000u);

    GP4_GLOBAL(uint8_t, 0x0095405e) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x1000u, 0x1000u));
}
GP4_IMPL(0x005278e5, FUN_005278e5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=pal_00527787+lut_0051c59a; maxinsn=20000000; wall=3600")
