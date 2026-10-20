#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00538e1a
// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell):
//   5  store (id - 1) in the value cell (0x00539a13), id being the byte +0xc of the item;
//   1  clear bit 7 of the item flags (+0x1f); unless +0x1e has bit 7, compare the cell with (id - 1)
//      (0x005398c4) and set bit 7 of +0x1f again when equal; edx comes back 0;
//   other commands do nothing (flags of `cmp edx, 5`).
// eax is (id - 1) after the compare/store; the flags are those of the last writer on the path.
void __cdecl FUN_00538e1a(gp4::Regs* r) {
    const uint32_t command = r->edx;
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if (command == 5u) {
        gp4::Regs c = *r;
        c.eax = static_cast<uint32_t>(GP4_FIELD(uint8_t, item, 0xc)) - 1u;
        gp4::call_regs(0x00539a13, c);
        r->eax = c.eax;
        r->eflags = c.eflags;
        return;
    }
    if (command != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 5u));
        return;
    }

    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
    GP4_FIELD(uint8_t, item, 0x1f) = cleared;
    uint32_t arith = gp4::flags::logic8(GP4_FIELD(uint8_t, item, 0x1e) & 0x80u);
    if ((GP4_FIELD(uint8_t, item, 0x1e) & 0x80u) == 0u) {
        gp4::Regs c = *r;
        c.eax = static_cast<uint32_t>(GP4_FIELD(uint8_t, item, 0xc)) - 1u;
        gp4::call_regs(0x005398c4, c);
        r->eax = c.eax;
        arith = c.eflags;
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00538e1a, FUN_00538e1a, "regs(eax:u32, edx:u32{1,1,1,5,5,0,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of)")
