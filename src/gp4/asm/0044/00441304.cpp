#include <gp4/gp4.h>

struct Unknown_Car {
    uint8_t Unknown00[0x7c];
    uint8_t Number7C;
    uint8_t Unknown7D[2];
    uint8_t Level7F;
    uint8_t Unknown80[0xe5 - 0x80];
    uint8_t FlagsE5;
};

// Row of the per-car tables for a car number byte: (number & 0x3f) - 1 as `dec al; and eax, 0x3f`
// (wrapping) then capped at 0x15.
static inline uint32_t wrappedRow(uint8_t number) {
    const uint32_t row = (static_cast<uint32_t>(number) - 1u) & 0x3fu;
    return static_cast<int32_t>(row) > 0x15 ? 0x15u : row;
}

// pushal / popal: picks the driver-level row for the car in esi and records the column the car's best entry
// sits in. When bit 2 of +0xe5 is set the level comes from the gear-band computation (0x0043ca66 / 0x0043ca8b /
// 0x0043c4d9; the band when it is at least 2, else the 0/1 of 0x0043c475) and the interpolated value is kept
// in [0x006227a0]; otherwise it is the byte +0x7f. 0x004411f7 then searches the column for the car row from
// the level table entry [0x006f2cc4 + 4 * level], the column byte is stored in 0x0066d936[row] and 0x004415c9
// refreshes the car. Registers are restored; the flags are those 0x004415c9 leaves.
// FUNCTION: GP4 0x00441304
void __cdecl FUN_00441304(gp4::Regs* r) {
    Unknown_Car* car = reinterpret_cast<Unknown_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = wrappedRow(car->Number7C);
    c.ecx = car->Level7F;

    if ((car->FlagsE5 & 4u) != 0u) {
        const uint32_t savedRow = c.eax;

        int32_t row = static_cast<int32_t>(car->Number7C & 0x3fu) - 1;
        if (row > 0x15) row = 0x15;
        c.eax = static_cast<uint32_t>(row);
        gp4::call_regs(0x0043ca66, c);
        GP4_GLOBAL(uint32_t, 0x006227a0) = c.eax;
        gp4::call_regs(0x0043ca8b, c);
        gp4::call_regs(0x0043c4d9, c);
        c.edx = c.eax;
        c.eax = savedRow;

        if (static_cast<int32_t>(c.edx) >= 2) {
            c.ecx = c.edx;
        } else {
            gp4::call_regs(0x0043c475, c);
            c.ecx = c.eax;
            c.eax = savedRow;
        }
    }

    c.eax = wrappedRow(car->Number7C);
    c.edx = GP4_ARRAY(const uint32_t, 0x006f2cc4)[c.ecx];
    gp4::call_regs(0x004411f7, c);
    GP4_ARRAY(uint8_t, 0x0066d936)[c.eax] = static_cast<uint8_t>(c.edx);
    gp4::call_regs(0x004415c9, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00441304, FUN_00441304, "regs(esi:ptr[0x240]:layout=lvl_00431d1f) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=levels_006f2cc4+tbl_0066d94c+trk_007ad894+circ_007adb50+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+counter_0040f0dd+tabs_004413b7+scale_0066ec60")
