#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell, the
// mask is the dword +0x26 of the item, the first dword of the item points at the next item):
//   2     set the byte [0x0082fd5f] = 0xff, read the cell (0x00539944), flip the mask bits in it and write it
//         back (0x00539a13); edi = the item's first dword (the linked item): its flag byte +0x1f gets bit 6
//         set, bit 5 cleared, and bit 5 set again unless the low byte of the stored value is 0xff; then
//         continue like command 1;
//   1     clear bit 7 of the item flags (+0x1f), read the cell and set bit 7 again when all mask bits are
//         set in it (eax comes back as cell & mask);
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command; the flags are those of the last writer on the path.
// FUNCTION: GP4 0x0052eadf
void __cdecl FUN_0052eadf(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;
    bool tail = false;

    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
        tail = true;
    } else if (command == 2u) {
        GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x00539944, c);
        c.eax ^= GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x00539a13, c);
        r->eax = c.eax;
        const uint32_t linked = GP4_FIELD(uint32_t, item, 0);
        r->edi = linked;
        GP4_FIELD(uint8_t, linked, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, linked, 0x1f) | 0x40u);
        GP4_FIELD(uint8_t, linked, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, linked, 0x1f) & 0xdfu);
        if ((c.eax & 0xffu) != 0xffu) {
            GP4_FIELD(uint8_t, linked, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, linked, 0x1f) | 0x20u);
        }
        arith = 0u;
        tail = true;
    } else if (command == 5u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        arith = gp4::flags::sub32(command, 5u);
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    if (tail) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x00539944, c);
        const uint32_t mask = GP4_FIELD(uint32_t, item, 0x26);
        const uint32_t value = c.eax & mask;
        r->eax = value;
        arith = gp4::flags::sub32(value, mask);
        if (value == mask) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052eadf, FUN_0052eadf, "regs(eax:u32, ebx:u32{=0x81e4f8,0x7ab544}, edi:u32, edx:u32{1,2,5,6,0,1,2}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa}) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+mask_0052eadf")
