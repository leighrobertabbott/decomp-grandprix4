#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell):
//   2     read the cell (0x00539944) and write it back (0x00539a13) as 0 when it was non-zero or as -1 when
//         it was zero, then continue as command 1;
//   1     clear bit 7 of the item flags (+0x1f), read the cell again and set bit 7 when it is non-zero;
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command; the flags are those of the last writer on the path.
// FUNCTION: GP4 0x005390d5
void __cdecl FUN_005390d5(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;
    bool check = false;

    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
        check = true;
    } else if (command == 2u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x00539944, c);
        c.eax = (c.eax != 0u) ? 0u : 0xffffffffu;
        gp4::call_regs(0x00539a13, c);
        r->eax = c.eax;
        arith = c.eflags;
        check = true;
    } else if (command == 5u) {
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        arith = gp4::flags::sub32(command, 5u);
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    if (check) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x00539944, c);
        const uint32_t value = c.eax;
        r->eax = value;
        arith = gp4::flags::logic32(value);
        if (value != 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005390d5, FUN_005390d5, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
