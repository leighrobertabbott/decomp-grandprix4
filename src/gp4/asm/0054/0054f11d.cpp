#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item handler selected by the command in edx (esi = the item, ebx = its value cell, the compared and stored
// value is the dword +0x26 of the item):
//   1     clear bit 7 of the item flags (+0x1f), compare the cell with the value (0x005398c4) and set bit 7 again
//         when they are equal;
//   2     store the value into the cell (0x00539a13), then the screen refresh 0x0054f54b and the circuit
//         entries 0x0054f169;
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command, eax is the value for commands 1 and 2; the flags are those of the last
// writer on the path taken.
// FUNCTION: GP4 0x0054f11d
void __cdecl FUN_0054f11d(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;

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
        arith = c.eflags;
        if ((c.eflags & gp4::ZF) != 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
    } else if (command == 2u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x00539a13, c);
        gp4::call_regs(0x0054f54b, c);
        gp4::call_regs(0x0054f169, c);
        r->eax = c.eax;
        arith = c.eflags;
    } else if (command == 5u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        arith = gp4::flags::sub32(command, 5u);
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054f11d, FUN_0054f11d, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894")
