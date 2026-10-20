#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the 254-entry colour index table at 0x00934cd8 (entries 2..0xff): for each point {x, y, z} of the
// 6-byte entries of the palette table 0x007c4d5c, the coordinates are scaled by [0x00934f28] (imul / shrd 14,
// low 32 bits of (c * scale) >> 14) and offset by [0x00934f24]; their low words go to 0x00933a14/16/18 and the
// nearest-point search 0x0051b645 (with [0x0095405e] = 0, every entry measured, table pointer 0x0096a3d4 =
// 0x007c4d5c) gives the entry stored at [0x00934cd8 + index]. pushal/popal restore every register; the flags
// are those of the final `cmp edi, 0x100`.
// FUNCTION: GP4 0x00550a1e
void __cdecl FUN_00550a1e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x0095405e) = 0u;
    GP4_GLOBAL(uint32_t, 0x0096a3d4) = 0x007c4d5cu;
    const uint16_t* const palette = GP4_ARRAY(uint16_t, 0x007c4d5c);
    uint8_t* const colours = GP4_ARRAY(uint8_t, 0x00934cd8);

    for (uint32_t index = 2u; index != 0x100u; ++index) {
        const uint16_t* point = palette + index * 3u;
        const int32_t scale = GP4_GLOBAL(int32_t, 0x00934f28);

        const uint32_t x = static_cast<uint32_t>(
            static_cast<uint32_t>((static_cast<int64_t>(point[0]) * scale) >> 14) + GP4_GLOBAL(uint32_t, 0x00934f24));
        GP4_GLOBAL(uint16_t, 0x00933a14) = static_cast<uint16_t>(x);

        const uint32_t y = static_cast<uint32_t>(
            static_cast<uint32_t>((static_cast<int64_t>(point[1]) * scale) >> 14) + GP4_GLOBAL(uint32_t, 0x00934f24));
        GP4_GLOBAL(uint16_t, 0x00933a16) = static_cast<uint16_t>(y);

        const uint32_t z = static_cast<uint32_t>(
            static_cast<uint32_t>((static_cast<int64_t>(point[2]) * scale) >> 14) + GP4_GLOBAL(uint32_t, 0x00934f24));
        GP4_GLOBAL(uint16_t, 0x00933a18) = static_cast<uint16_t>(z);

        c.eax = z;
        gp4::call_regs(0x0051b645, c);
        colours[index] = static_cast<uint8_t>(c.eax);
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x100u, 0x100u));
}
GP4_IMPL(0x00550a1e, FUN_00550a1e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; cases=24; wall=3400")
