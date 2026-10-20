#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell, the stored
// value is the dword +0x26 of the item), the variant of 0x00538e95 that also refreshes the menu pages:
//   1     clear bit 7 of the item flags (+0x1f), compare the cell with the dword +0x26 (0x005398c4) and set
//         bit 7 again when equal;
//   2, 3  run 0x00538e95 (command 2 stores the dword +0x26 into the cell), then refresh the menu pages
//         (0x00529408, 0x0052a545, 0x00528927, 0x005413e9);
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command; the flags are those of the last writer on the path.
// FUNCTION: GP4 0x00528757
void __cdecl FUN_00528757(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;

    if (command == 1u) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x005398c4, c);
        r->eax = c.eax;
        uint32_t arith = c.eflags;
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    if (command == 2u || command == 3u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00538e95, c);
        gp4::call_regs(0x00529408, c);
        gp4::call_regs(0x0052a545, c);
        gp4::call_regs(0x00528927, c);
        gp4::call_regs(0x005413e9, c);
        c.edx = 0u;
        *r = c;
        return;
    }

    uint32_t arith;
    if (command == 5u) {
        arith = gp4::flags::sub32(command, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00528757, FUN_00528757, "regs(eax:u32, edx:u32{1,2,3,2,3,0,5,6}, ebx:ptr[8]:layout=cell_005292c0, esi:ptr[0x40]:layout=item_00529148) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=session_004fd1a0+cobj_0084e9bb+trk_007ad894+ramp_0081d1f2+menu_00529408")
