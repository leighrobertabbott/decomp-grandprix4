#include <gp4/gp4.h>
#include <gp4/flags.h>

// Command handler (edx = command, esi = menu item record) of the save-slot list menu around 0x007ab1a4.
// Command 2 (select): slot = item id (+0xc) - 10. When it differs from the current slot byte [0x007ab4a4] it
// becomes the current slot; when it is the current slot, [0x0081e4f8] becomes 2 if the slot's bytes +0x00 and
// +0x20 are both zero, else 0, with [0x0081e4fa] = 0xff. Then the slot flags are refreshed through 0x0053f3a7
// and [0x0081e4fb] = 0xff. The command falls through into command 1 (refresh): clear bit 7 of the item flag
// byte +0x1f and, when the low byte of [0x0082fd5a] equals [0x0081e4f8], compare the dword at item +0x26 with
// the cell at [ebx] through 0x005398c4 (width from the item's byte +0xd); equal sets bit 7 of +0x1f again.
// edx comes back 0 for both commands; any other command does nothing (the flags are those of `cmp edx, 2`).
// FUNCTION: GP4 0x0053f432
void __cdecl FUN_0053f432(gp4::Regs* r) {
    const uint32_t command = r->edx;
    if (command != 1u && command != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 2u));
        return;
    }

    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    if (command == 2u) {
        const uint32_t ebx = static_cast<uint32_t>(item[0xc]) - 0x0au;
        const uint32_t slotIndex = ebx & 0xffu;
        const uint32_t current = GP4_GLOBAL(uint8_t, 0x007ab4a4);
        uint32_t arith = gp4::flags::sub8(slotIndex, current);
        c.ebx = ebx;
        if (slotIndex != current) {
            GP4_GLOBAL(uint8_t, 0x007ab4a4) = static_cast<uint8_t>(slotIndex);
        } else {
            const uint32_t slotAddr = ebx * 0x40u + 0x007ab1a4u;
            const uint8_t* slot = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(slotAddr));
            c.eax = slotAddr;
            if (slot[0x20] != 0u) {
                arith = gp4::flags::sub8(slot[0x20], 0u);
                GP4_GLOBAL(uint8_t, 0x0081e4f8) = 0u;
            } else {
                arith = gp4::flags::sub8(slot[0], 0u);
                GP4_GLOBAL(uint8_t, 0x0081e4f8) = (slot[0] != 0u) ? 0u : 2u;
            }
            GP4_GLOBAL(uint8_t, 0x0081e4fa) = 0xffu;
        }
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x0053f3a7, c);
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
    }

    item[0x1f] = static_cast<uint8_t>(item[0x1f] & 0x7fu);
    c.eax = GP4_GLOBAL(uint32_t, 0x0082fd5a);
    const uint32_t al = c.eax & 0xffu;
    uint32_t arith = gp4::flags::sub8(al, GP4_GLOBAL(uint8_t, 0x0081e4f8));
    if (al == GP4_GLOBAL(uint8_t, 0x0081e4f8)) {
        c.eax = *reinterpret_cast<const uint32_t*>(item + 0x26);
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x005398c4, c);
        arith = c.eflags;
        if ((c.eflags & gp4::flags::ZF) != 0u) {
            item[0x1f] = static_cast<uint8_t>(item[0x1f] | 0x80u);
            arith = gp4::flags::logic8(item[0x1f]);
        }
    }
    c.edx = 0u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    *r = c;
}
GP4_IMPL(0x0053f432, FUN_0053f432, "regs(ebx:ptr[8]:u8{0,0x1a,0x40,0x66,0x8c,0xb2,0xd8,0xfe,0x24,0x4a,0x70,6}, edx:u32{1,1,2,2,5,0}, esi:u32{=0x8481ce,0x8481f4,0x84821a,0x848240,0x848266,0x84828c,0x8482b2,0x8482d8,0x8482fe,0x848324,0x84834a,0x848370}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+slots_007ab1a4+flag4_005394b1+sel_0053f432")
