#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the grid descriptor selected by the index [0x66d8d8] (via 0x0052197e), then reads the
// cursor-dependent entries from it: the two bytes at +2+2*col go to 0x893cf4/0x893cf5, the row
// cursor [0x66d8dc] is clamped between two per-column byte tables and stored back, and the four
// bytes of the cell at (row - low) are copied to 0x893cf0/0x893cf2/0x893cf1/0x893cf3. pushal/popal
// keeps every register; the flags are those of the final address add.
// FUNCTION: GP4 0x005219bb
void __cdecl FUN_005219bb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ebx = GP4_GLOBAL(uint32_t, 0x0066d8d8);
    gp4::call_regs(0x0052197e, c);

    const uint32_t desc = c.ebx;
    const uint32_t col = GP4_GLOBAL(uint32_t, 0x0066d8e0);

    const uint32_t cell_stride = (static_cast<uint32_t>(GP4_ARRAY(uint8_t, desc)[0]) << 2) * col;
    const uint32_t width = GP4_ARRAY(uint8_t, desc)[1];

    uint32_t p = desc + 2u;
    GP4_GLOBAL(uint8_t, 0x00893cf4) = GP4_ARRAY(uint8_t, p)[col * 2u];
    GP4_GLOBAL(uint8_t, 0x00893cf5) = GP4_ARRAY(uint8_t, p)[col * 2u + 1u];

    p += width;
    p += width;
    const uint32_t low = GP4_ARRAY(uint8_t, p)[col];
    uint32_t row = GP4_GLOBAL(uint32_t, 0x0066d8dc);
    if (static_cast<int32_t>(row) < static_cast<int32_t>(low)) {
        row = low;
    }
    p += width;
    const uint32_t high = GP4_ARRAY(uint8_t, p)[col];
    if (static_cast<int32_t>(row) > static_cast<int32_t>(high)) {
        row = high;
    }
    GP4_GLOBAL(uint32_t, 0x0066d8dc) = row;

    const uint32_t row_offset = (row - low) << 2;
    p += width;
    p += cell_stride;
    const uint32_t arith = gp4::flags::add32(p, row_offset);
    p += row_offset;

    GP4_GLOBAL(uint8_t, 0x00893cf0) = GP4_ARRAY(uint8_t, p)[0];
    GP4_GLOBAL(uint8_t, 0x00893cf2) = GP4_ARRAY(uint8_t, p)[1];
    GP4_GLOBAL(uint8_t, 0x00893cf1) = GP4_ARRAY(uint8_t, p)[2];
    GP4_GLOBAL(uint8_t, 0x00893cf3) = GP4_ARRAY(uint8_t, p)[3];

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005219bb, FUN_005219bb, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=links_00521d6f+carbg_0066fef0+state_004059ef+player_0040f8d9+car_0052197e+cursor_005219bb")
