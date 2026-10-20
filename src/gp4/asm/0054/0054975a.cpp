#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler for the command in edx (esi = the item, ebx = the cell it edits).
// Command 1 clears bit 7 of the item flag byte +0x1f, compares the cell with the item's dword +0x26
// (0x005398c4, eax = that dword) and sets bit 7 again when they are equal. Command 2 stores the dword +0x26
// into the cell (0x00539a13) and runs the dispatcher 0x005497af, then sets the bytes [0x0081e4fb] = 0xff and
// [0x0081e4fe] = 0. Commands 5 and 6 clear the dword [0x0082fd88]. edx comes back 0 on every path; the
// flags are those of the last writer on the path.
// FUNCTION: GP4 0x0054975a
void __cdecl FUN_0054975a(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;

    if (command == 1u) {
        const uint32_t cleared = GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu;
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(cleared);
        r->eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x005398c4, c);
        arith = c.eflags;
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            const uint32_t marked = GP4_FIELD(uint8_t, item, 0x1f) | 0x80u;
            GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(marked);
            arith = gp4::flags::logic8(marked);
        }
    } else if (command == 2u) {
        r->eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x00539a13, *r);
        gp4::call_regs(0x005497af, *r);
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x0081e4fe) = 0u;
        r->edx = 0u;
        return;
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
GP4_IMPL(0x0054975a, FUN_0054975a, "regs(eax:u32, ebx:ptr[8]:u8{0,0,1,2}, edx:u32{1,2,5,6,0,3,4,1,2}, esi:ptr[0x40]:u8{0,0,1,2}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
