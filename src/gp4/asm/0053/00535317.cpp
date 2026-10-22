#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the event in edx (esi = the item, ebx = its value cell, the stored
// value is the dword +0x26 of the item), twin of 0x00538e95 whose event 2 also refreshes the menu:
//   1     clear bit 7 of the item flags (+0x1f), compare the cell with the dword +0x26 (0x005398c4) and
//         set bit 7 again when equal;
//   2     store the dword +0x26 into the cell (0x00539a13), then run 0x005350e1;
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every event; the flags are those of the last writer on the path, eax the dword
// +0x26 for the events 1 and 2.
// FUNCTION: GP4 0x00535317
void __cdecl FUN_00535317(gp4::Regs* r) {
    const uint32_t event = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    if (event == 1u) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x005398c4, c);
        arith = c.eflags;
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
    } else if (event == 2u) {
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x00539a13, c);
        gp4::call_regs(0x005350e1, c);
        arith = c.eflags;
    } else if (event == 5u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        arith = gp4::flags::sub32(event, 5u);
    } else {
        arith = gp4::flags::sub32(event, 6u);
        if (event == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    c.edx = 0u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    *r = c;
}
GP4_IMPL(0x00535317, FUN_00535317, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=idx_00535239+sel_0053aab9+mode_007aafd3")
