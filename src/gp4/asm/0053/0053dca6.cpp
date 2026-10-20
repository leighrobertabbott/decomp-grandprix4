#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* menuPtr(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// Menu navigation: follows one direction link of the menu item in esi. ebx is the byte offset of the
// link inside the item (0x22..0x25, one per direction), so `[ebx + item]` is the 1-based index of the
// neighbouring item (0 = none, 0xfa/0xfb = special commands). A link to item k hops k times along the
// chain that starts at [0x0082fd34] (k must not exceed the item count dword 0x0082fd52). If that item
// is disabled (flag byte +0x1f has 0x10 or 0x20) the walk continues with the neighbour's own link at
// the same offset, otherwise bit 0x40 is set in +0x1f of both esi's item and the target, and the
// selection byte 0x0082fd45 takes the index. Link 0xfa/0xfb is only honoured while the walk is still
// at the entry item and calls the handler stored at 0x0082feac + 4 * (link - 0xfa); anything else
// (index above the count, other special codes) runs the empty routine 0x0054e232 inside a pushal/popal.
// eax is the link byte (0 for none; the table slot address after a special command, link - 0xfa when
// the special code was rejected), ecx is 0 once a hop loop ran and edi is the item reached last.
// The flags are those of the last flag writer on the taken path.
// FUNCTION: GP4 0x0053dca6
void __cdecl FUN_0053dca6(gp4::Regs* r) {
    const uint32_t ebx = r->ebx;
    const uint32_t esi = r->esi;
    uint32_t ecx = r->ecx;
    uint32_t edi = esi;
    uint32_t eax = 0u;
    uint32_t arith = 0u;
    bool rejected = false;

    for (;;) {
        eax = GP4_FIELD(uint8_t, menuPtr(edi), ebx);
        arith = gp4::flags::logic32(eax);
        if (eax == 0u) {
            break;
        }
        arith = gp4::flags::sub32(eax, 0xfau);
        if (eax >= 0xfau) {
            arith = gp4::flags::sub32(esi, edi);
            if (esi != edi) {
                rejected = true;
                break;
            }
            eax -= 0xfau;
            arith = gp4::flags::sub32(eax, 2u);
            if (eax >= 2u) {
                rejected = true;
                break;
            }
            const uint32_t slot = 0x0082feacu + (eax << 2);
            const uint32_t handler = GP4_FIELD(uint32_t, menuPtr(slot), 0);
            gp4::Regs c = *r;
            c.eax = slot;
            c.ebx = ebx;
            c.ecx = ecx;
            c.esi = esi;
            c.edi = edi;
            gp4::call_regs(handler, c);
            r->eax = c.eax;
            r->ebx = c.ebx;
            r->ecx = c.ecx;
            r->edx = c.edx;
            r->esi = c.esi;
            r->edi = c.edi;
            r->ebp = c.ebp;
            r->eflags = gp4::flags::merge(r->eflags, c.eflags);
            return;
        }
        const uint32_t count = GP4_GLOBAL(uint32_t, 0x0082fd52);
        arith = gp4::flags::sub32(eax, count);
        if (eax > count) {
            rejected = true;
            break;
        }
        uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
        for (uint32_t hops = eax; hops != 0u; --hops) {
            node = GP4_FIELD(uint32_t, menuPtr(node), 0);
        }
        ecx = 0u;
        edi = node;
        const uint8_t state = GP4_FIELD(uint8_t, menuPtr(edi), 0x1f);
        if ((state & 0x10u) != 0u) {
            continue;
        }
        if ((state & 0x20u) != 0u) {
            continue;
        }
        GP4_FIELD(uint8_t, menuPtr(esi), 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, menuPtr(esi), 0x1f) | 0x40u);
        const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, menuPtr(edi), 0x1f) | 0x40u);
        GP4_FIELD(uint8_t, menuPtr(edi), 0x1f) = marked;
        arith = gp4::flags::logic8(marked);
        GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(eax);
        break;
    }

    if (rejected) {
        gp4::cdecl_<0x0054e232, void>();
    }
    r->eax = eax;
    r->ecx = ecx;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053dca6, FUN_0053dca6, "regs(ebx:u32{=0x22,0x23,0x24,0x25}, ecx:u32, edx:u32, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053d8c5+items_0053dc57+links_0053dca6")
