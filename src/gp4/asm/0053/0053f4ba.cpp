#include <gp4/gp4.h>
#include <gp4/flags.h>

// Command handler (edx = command, esi = menu item record) of the slot-list menu around 0x007ab1a4.
// Command 1: clear bit 7 of the item flag byte +0x1f; when bit 7 of the state byte +0x1e is clear, compare
// (id - 1), id = byte +0xc, with the cell at [ebx] through 0x005398c4 (width from the item's byte +0xd) and
// set bit 7 of +0x1f again when they are equal. edx comes back 0.
// Command 5: when the slot index byte [0x007ab4a4] is not 0, exchange the 0x40-byte slot of that index with
// the slot before it (esi/edi/ecx/eax are left as the byte-swap loop leaves them), then decrement the index
// byte, refresh the slot flags through 0x0053f3a7 and set [0x0081e4fb] = 0xff. Any other command does nothing
// (the flags are those of `cmp edx, 5`).
// FUNCTION: GP4 0x0053f4ba
void __cdecl FUN_0053f4ba(gp4::Regs* r) {
    const uint32_t command = r->edx;
    uint32_t arith;

    if (command == 1u) {
        uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
        item[0x1f] = static_cast<uint8_t>(item[0x1f] & 0x7fu);
        arith = gp4::flags::logic8(item[0x1f]);
        const uint8_t state = item[0x1e];
        arith = gp4::flags::logic8(state & 0x80u);
        if ((state & 0x80u) == 0u) {
            const uint32_t id = item[0xc];
            gp4::Regs c = *r;
            c.eax = id - 1u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(id, arith));
            gp4::call_regs(0x005398c4, c);
            r->eax = c.eax;
            arith = c.eflags;
            if ((c.eflags & gp4::flags::ZF) != 0u) {
                item[0x1f] = static_cast<uint8_t>(item[0x1f] | 0x80u);
                arith = gp4::flags::logic8(item[0x1f]);
            }
        }
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, arith);
    } else if (command == 5u) {
        const uint32_t index = GP4_GLOBAL(uint8_t, 0x007ab4a4);
        gp4::Regs c = *r;
        c.eax = index;
        arith = gp4::flags::logic32(index);
        if (index != 0u) {
            const uint32_t slot = index * 0x40u + 0x007ab1a4u;
            uint8_t* cur = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(slot));
            uint8_t* prev = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(slot - 0x40u));
            uint32_t lo = 0u;
            uint32_t hi = 0u;
            for (uint32_t i = 0u; i < 0x40u; ++i) {
                lo = cur[i];
                hi = prev[i];
                cur[i] = static_cast<uint8_t>(hi);
                prev[i] = static_cast<uint8_t>(lo);
            }
            c.eax = ((slot - 0x40u) & 0xffff0000u) | (hi << 8) | lo;
            c.esi = slot + 0x40u;
            c.edi = slot;
            c.ecx = 0u;
            arith = gp4::flags::dec32(1u, 0u);
        }
        c.eflags = gp4::flags::merge(c.eflags, arith);
        GP4_GLOBAL(uint8_t, 0x007ab4a4) = static_cast<uint8_t>(index - 1u);
        gp4::call_regs(0x0053f3a7, c);
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
        *r = c;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 5u));
    }
}
GP4_IMPL(0x0053f4ba, FUN_0053f4ba, "regs(ebx:ptr[8]:u8{0,0,0,0,0,1,2,3}, edx:u32{1,1,1,1,5,0,2}, esi:u32{=0x843040,0x843040,0x843066,0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+slots_007ab1a4+flag4_005394b1")
