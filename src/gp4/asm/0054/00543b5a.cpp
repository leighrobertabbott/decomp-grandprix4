#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scroll/zoom menu item handler selected by edx (the esi record is the menu item).
//  edx == 1: clears bit 7 of the item byte +0x1f; unless bit 7 of +0x1e is set, the item id byte
//            +0xc minus one is compared with the value at [ebx] (0x005398c4, width from the item's
//            +0xd) and bit 7 of +0x1f is set again on equality. edx becomes 0, eax the id - 1.
//  edx == 5: when both view blocks are at their defaults (positions [0x008310c0] and [0x008310cc]
//            zero, zoom words [0x0083137b] and [0x0083137f] 0x4000) the plot corners are recomputed
//            by 0x005426d3, otherwise both view blocks are reset by 0x00543be3 and 0x00543c01.
//  other edx: nothing happens.
// The flags are those of the last flag writer on the taken path.
// FUNCTION: GP4 0x00543b5a
void __cdecl FUN_00543b5a(gp4::Regs* r) {
    const uint32_t edx = r->edx;
    uint32_t arith = gp4::flags::sub32(edx, 1u);

    if (edx == 1u) {
        const uint32_t record = r->esi;
        uint32_t flag_byte = GP4_FIELD(uint8_t, record, 0x1f) & 0x7fu;
        GP4_FIELD(uint8_t, record, 0x1f) = static_cast<uint8_t>(flag_byte);
        arith = gp4::flags::logic8(flag_byte);
        arith = gp4::flags::logic8(GP4_FIELD(uint8_t, record, 0x1e) & 0x80u);
        if ((GP4_FIELD(uint8_t, record, 0x1e) & 0x80u) == 0u) {
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eax = static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 0xc)) - 1u;
            gp4::call_regs(0x005398c4, c);
            arith = c.eflags;
            r->eax = c.eax;
            r->ebx = c.ebx;
            r->ecx = c.ecx;
            r->esi = c.esi;
            r->edi = c.edi;
            r->ebp = c.ebp;
            if ((arith & gp4::flags::ZF) != 0u) {
                flag_byte = GP4_FIELD(uint8_t, record, 0x1f) | 0x80u;
                GP4_FIELD(uint8_t, record, 0x1f) = static_cast<uint8_t>(flag_byte);
                arith = gp4::flags::logic8(flag_byte);
            }
        }
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    arith = gp4::flags::sub32(edx, 5u);
    if (edx != 5u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    bool defaults = true;
    const uint32_t x_pos = GP4_GLOBAL(uint32_t, 0x008310c0);
    arith = gp4::flags::sub32(x_pos, 0u);
    if (x_pos != 0u) {
        defaults = false;
    }
    if (defaults) {
        const uint32_t x_zoom = GP4_GLOBAL(uint32_t, 0x0083137b);
        arith = gp4::flags::sub32(x_zoom, 0x4000u);
        if (x_zoom != 0x4000u) {
            defaults = false;
        }
    }
    if (defaults) {
        const uint32_t y_pos = GP4_GLOBAL(uint32_t, 0x008310cc);
        arith = gp4::flags::sub32(y_pos, 0u);
        if (y_pos != 0u) {
            defaults = false;
        }
    }
    if (defaults) {
        const uint32_t y_zoom = GP4_GLOBAL(uint32_t, 0x0083137f);
        arith = gp4::flags::sub32(y_zoom, 0x4000u);
        if (y_zoom != 0x4000u) {
            defaults = false;
        }
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(r->eflags, arith);
    if (defaults) {
        gp4::call_regs(0x005426d3, c);
    } else {
        gp4::call_regs(0x00543be3, c);
        gp4::call_regs(0x00543c01, c);
    }
    *r = c;
}
GP4_IMPL(0x00543b5a, FUN_00543b5a, "regs(edx:u32{1,5,1,5,0,2}, ebx:ptr[8]:u8{0,0,1,2,0xff}, esi:ptr[0x40]:u8{0,0,1,2,3,0x80,0xff}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=scroll_00543340+view_00543b5a")
