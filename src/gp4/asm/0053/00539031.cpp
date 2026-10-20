#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell, the
// mask is the dword +0x26 of the item):
//   1     clear bit 7 of the item flags (+0x1f), read the cell (0x00539944) and set bit 7 again when none
//         of the mask bits are set in it;
//   2     read the cell, clear the mask bits in it and write it back (0x00539a13);
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command; the flags are those of the last writer on the path.
// FUNCTION: GP4 0x00539031
void __cdecl FUN_00539031(gp4::Regs* r) {
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
        gp4::call_regs(0x00539944, c);
        const uint32_t value = c.eax & GP4_FIELD(uint32_t, item, 0x26);
        r->eax = value;
        arith = gp4::flags::logic32(value);
        if (value == 0u) {
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
        gp4::call_regs(0x00539944, c);
        const uint32_t mask = GP4_FIELD(uint32_t, item, 0x26);
        c.eax = (c.eax | mask) ^ mask;
        gp4::call_regs(0x00539a13, c);
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
GP4_IMPL(0x00539031, FUN_00539031, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
