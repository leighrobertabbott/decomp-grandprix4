#include <gp4/gp4.h>

// Re-picks the tyre-compound column of the car in esi (car number byte +0x7c, driver level byte +0x7f), then
// refreshes the car's derived values with 0x004415c9.
//   * Bit 2 of +0x230 set: clear it, set bit 2 of +0x231, copy the level +0x160 into +0x7f and re-pick.
//   * Otherwise a level below 2 keeps the current column [0x66d936 + row] while its offset inside its level group
//     ([0x6f2c6a + column]) is below 7, moving it one step forward; a level of 2 or more (or an offset of 7 or
//     more) asks 0x004411f7 for the best column of the level group [0x6f2cc4 + 4 * level].
// row = ((+0x7c - 1) & 0x3f) clamped to 0x15. eax ends as the row, edx as the stored column (or the value returned
// by 0x004411f7); the flags are those the final 0x004415c9 leaves.
// FUNCTION: GP4 0x0044126d
void __cdecl FUN_0044126d(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t row = static_cast<uint32_t>(static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x7c) - 1u)) & 0x3fu;
    if (static_cast<int32_t>(row) > 0x15) {
        row = 0x15u;
    }

    uint32_t level = GP4_FIELD(uint8_t, car, 0x7f);
    bool repick = false;
    uint32_t stored = 0u;
    if ((GP4_FIELD(uint8_t, car, 0x230) & 4u) != 0u) {
        GP4_FIELD(uint8_t, car, 0x230) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x230) & 0xfbu);
        GP4_FIELD(uint8_t, car, 0x231) = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x231) | 4u);
        level = GP4_FIELD(uint8_t, car, 0x160);
        GP4_FIELD(uint8_t, car, 0x7f) = static_cast<uint8_t>(level);
        repick = true;
    } else if (static_cast<int8_t>(level) >= 2) {
        repick = true;
    } else {
        const uint32_t column = GP4_ARRAY(uint8_t, 0x0066d936)[row];
        const uint8_t groupBase = GP4_ARRAY(uint8_t, 0x006f2c6a)[column];
        c.ecx = (c.ecx & 0xffffff00u) | groupBase;
        const uint32_t offset = static_cast<uint8_t>(column - groupBase);
        level = offset;
        if (static_cast<int32_t>(offset) >= 7) {
            repick = true;
        } else {
            stored = static_cast<uint8_t>(offset + 1u + groupBase);
            GP4_ARRAY(uint8_t, 0x0066d936)[row] = static_cast<uint8_t>(stored);
        }
    }

    c.eax = row;
    if (repick) {
        c.edx = GP4_ARRAY(uint32_t, 0x006f2cc4)[level];
        gp4::call_regs(0x004411f7, c);
        GP4_ARRAY(uint8_t, 0x0066d936)[c.eax] = static_cast<uint8_t>(c.edx);
    } else {
        c.edx = stored;
    }
    gp4::call_regs(0x004415c9, c);

    r->eax = c.eax;
    r->edx = c.edx;
    r->ebx = c.ebx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0044126d, FUN_0044126d, "regs(esi:ptr[1056]:layout=rec_0043259c) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_00410894+state_004087aa+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+state_0043c1ba+state_0043c4d9+state_004088c2+state_00408998+state_0043260e+state_004327cd+state_00432372+temp_00602b50+state_0040edc3+state_0040efa2+counter_0040f0dd+tabs_004413b7+scale_0066ec60")
