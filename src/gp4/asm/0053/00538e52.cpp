#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00538e52
// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell).
// Command 2 first copies the byte of the dword [0x0082fd5a] to the byte [0x0081e4f8] and stores the
// item's dword +0x26 into the cell (0x00539a13), then falls into command 1: clear bit 7 of the item
// flags (+0x1f); when the low byte of [0x0082fd5a] equals the byte [0x0081e4f8], compare the cell with
// the dword +0x26 (0x005398c4) and set bit 7 again when equal; edx comes back 0. Other commands do
// nothing (flags of `cmp edx, 2`). eax ends as [0x0082fd5a], or the +0x26 dword after a compare; the
// flags are those of the last writer on the path.
void __cdecl FUN_00538e52(gp4::Regs* r) {
    const uint32_t command = r->edx;
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if (command == 2u) {
        const uint32_t key = GP4_GLOBAL(uint32_t, 0x0082fd5a);
        GP4_GLOBAL(uint8_t, 0x0081e4f8) = static_cast<uint8_t>(key);
        gp4::Regs c = *r;
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
        gp4::call_regs(0x00539a13, c);
        r->eflags = c.eflags;
    } else if (command != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 2u));
        return;
    }

    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
    GP4_FIELD(uint8_t, item, 0x1f) = cleared;
    uint32_t arith = gp4::flags::logic8(cleared);

    const uint32_t key = GP4_GLOBAL(uint32_t, 0x0082fd5a);
    const uint8_t stored = GP4_GLOBAL(uint8_t, 0x0081e4f8);
    arith = gp4::flags::sub8(key, stored);
    r->eax = key;
    if (static_cast<uint8_t>(key) == stored) {
        gp4::Regs c = *r;
        c.eax = GP4_FIELD(uint32_t, item, 0x26);
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
GP4_IMPL(0x00538e52, FUN_00538e52, "regs(eax:u32, edx:u32{1,2,2,1,0,5}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
