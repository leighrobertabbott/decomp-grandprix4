#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell, the value is the
// dword +0x26 of the item):
//   2, 3  when the cell differs from the value (0x005398c4), store the value into it (0x00539a13); refresh the
//         menu pages (0x00529408, 0x0052a545, 0x005413e9) and fall into the command 1 body;
//   1     clear bit 7 of the item flags (+0x1f), compare the cell with the value (0x005398c4) and set bit 7 again
//         when equal;
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command; the registers and flags are what the last callee or flag writer on the path
// leaves.
static void mark_item(gp4::Regs& c) {
    const uint32_t item = c.esi;
    GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = GP4_FIELD(uint32_t, item, 0x26);
    gp4::call_regs(0x005398c4, c);
    if ((c.eflags & gp4::flags::ZF) != 0u) {
        const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
        GP4_FIELD(uint8_t, item, 0x1f) = marked;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(marked));
    }
}

// FUNCTION: GP4 0x005293ad
void __cdecl FUN_005293ad(gp4::Regs* r) {
    const uint32_t command = r->edx;

    if (command == 1u) {
        gp4::Regs c = *r;
        mark_item(c);
        c.edx = 0u;
        *r = c;
        return;
    }

    if (command == 2u || command == 3u) {
        gp4::Regs c = *r;
        c.eax = GP4_FIELD(uint32_t, c.esi, 0x26);
        gp4::call_regs(0x005398c4, c);
        if ((c.eflags & gp4::flags::ZF) == 0u) {
            gp4::call_regs(0x00539a13, c);
        }
        gp4::call_regs(0x00529408, c);
        gp4::call_regs(0x0052a545, c);
        gp4::call_regs(0x005413e9, c);
        mark_item(c);
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
GP4_IMPL(0x005293ad, FUN_005293ad, "regs(eax:u32, edx:u32{1,2,3,2,3,0,5,6}, ebx:ptr[8]:layout=cell_005292c0, esi:ptr[0x40]:layout=item_00529148) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=session_004fd1a0+trk_007ad894+ramp_0081d1f2+menu_00529408")
