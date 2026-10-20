#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws a menu item's frame (pushal / popal wrapper, every register comes back). Queues the item with the
// sorted-list insert 0x00549db9 (eax, ebx, ebp, ecx as the caller left them), then picks a style code in edx
// for the empty draw hook 0x00531b35: flag byte +0x1f bit 5 set gives 0x11, otherwise 0xf (0x10 when the
// item's id byte +0xc equals the selection byte [0x0082fd45]) after the empty inset hook 0x00531b3d. When
// bit 7 of +0x1f is set, a second pass with the box moved right by 3 picks 0x1c (bit 5) or 0x14 (0x15 on the
// id match, then 0x18 when [0x007aafd3] == 0xff and [0x007ab1a0], [0x007aafe0] are not 0xff) and calls the
// hook again. popal does not restore the flags: they are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x0053a1d4
void __cdecl FUN_0053a1d4(gp4::Regs* r) {
    const uint32_t record = r->esi;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00549db9, c);

    uint32_t bits = GP4_FIELD(uint8_t, record, 0x1f);
    uint32_t arith = gp4::flags::logic8(bits & 0x20u);
    if ((bits & 0x20u) != 0u) {
        c.edx = 0x11u;
    } else {
        gp4::Regs w = c;
        w.ebx += 2u;
        w.eax += 2u;
        w.ebp -= 2u;
        arith = gp4::flags::sub32(w.ecx, 2u);
        w.ecx -= 2u;
        w.eflags = gp4::flags::merge(w.eflags, arith);
        gp4::call_regs(0x00531b3d, w);

        c.edx = 0x0fu;
        const uint32_t id = GP4_FIELD(uint8_t, record, 0x0c);
        const uint32_t selected = GP4_GLOBAL(uint8_t, 0x0082fd45);
        const bool match = id == selected;
        arith = (gp4::flags::sub8(id, selected) & ~gp4::flags::CF) | (match ? gp4::flags::CF : 0u);
        if (match) {
            c.edx = 0x10u;
        }
    }
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x00531b35, c);

    bits = GP4_FIELD(uint8_t, record, 0x1f);
    arith = gp4::flags::logic8(bits & 0x80u);
    if ((bits & 0x80u) != 0u) {
        arith = gp4::flags::add32(c.ebx, 3u);
        c.ebx += 3u;
        arith = gp4::flags::add32(c.eax, 3u);
        c.eax += 3u;
        bits = GP4_FIELD(uint8_t, record, 0x1f);
        arith = gp4::flags::logic8(bits & 0x20u);
        if ((bits & 0x20u) != 0u) {
            c.edx = 0x1cu;
        } else {
            c.edx = 0x14u;
            const uint32_t id = GP4_FIELD(uint8_t, record, 0x0c);
            const uint32_t selected = GP4_GLOBAL(uint8_t, 0x0082fd45);
            const bool match = id == selected;
            arith = (gp4::flags::sub8(id, selected) & ~gp4::flags::CF) | (match ? gp4::flags::CF : 0u);
            if (match) {
                c.edx = 0x15u;
            }

            uint32_t carry = 0u;
            const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007aafd3);
            arith = gp4::flags::sub8(mode, 0xffu);
            if (mode == 0xffu) {
                const uint32_t second = GP4_GLOBAL(uint8_t, 0x007ab1a0);
                arith = gp4::flags::sub8(second, 0xffu);
                if (second != 0xffu) {
                    const uint32_t third = GP4_GLOBAL(uint8_t, 0x007aafe0);
                    arith = gp4::flags::sub8(third, 0xffu);
                    if (third != 0xffu) {
                        carry = gp4::flags::CF;
                    }
                }
            }
            arith = (arith & ~gp4::flags::CF) | carry;
            if (carry != 0u) {
                c.edx = 0x18u;
            }
        }
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00531b35, c);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053a1d4, FUN_0053a1d4, "regs(eax:u32, ebx:u32, ecx:u32, ebp:u32, esi:ptr[0x40]:u8{0,0x10,0x20,0x30,0x80,0x90,0xa0,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+sel_0053aab9")
