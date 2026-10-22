#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu input step, run only while bit 1 of the pending byte [0x0082fd6a] is set (and clearing that bit at
// the end). 0x0053d9a6 first turns the pointer event into a hit item id in [0x0082fd88]; with an item hit
// (ecx = id != 0) esi is the item (the id-th link of the menu chain) and edx the mode [0x0082fd9c]:
//  - mode 2 on an item whose +0x1e has bit 0x40, when the previously selected id [0x0082fd45] differs from
//    the hit id: the previous item (edi) and the hit item get +0x1f bit 0x40 and the hit id becomes the
//    selection;
//  - mode 5 on an item with +0x1e bit 0x80 sets the flag byte [0x0081e4fa];
//  - mode 2 also sets bit 7 of +0x1f (16-bit OR); the item's handler [+0x1a] runs with esi = item, ebx = the
//    item's dword +0xe (both restored afterwards), 0x0053dbbc applies the per-mode flag-byte update,
//    +0x1f bit 0x40 is set when the handler or 0x0053dbbc changed the byte, and 0x0053dc57 re-marks the
//    other items.
// Registers are whatever the callees leave (esi/ebx restored around the handler).
// FUNCTION: GP4 0x0053daae
void __cdecl FUN_0053daae(gp4::Regs* r) {
    const uint32_t pending = GP4_GLOBAL(uint8_t, 0x0082fd6a);
    if ((pending & 2u) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(pending & 2u));
        return;
    }

    gp4::Regs c = *r;
    gp4::call_regs(0x0053d9a6, c);
    c.edx = GP4_GLOBAL(uint32_t, 0x0082fd9c);
    c.ecx = GP4_GLOBAL(uint32_t, 0x0082fd88);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 0u));

    if (c.ecx != 0u) {
        gp4::call_regs(0x0053d98c, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edx, 2u));
        if (c.edx == 2u) {
            const uint32_t itemFlags = GP4_FIELD(uint8_t, c.esi, 0x1e) & 0x40u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(itemFlags));
            if (itemFlags != 0u) {
                c.ecx = GP4_GLOBAL(uint8_t, 0x0082fd45);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, GP4_GLOBAL(uint32_t, 0x0082fd88)));
                if (c.ecx != GP4_GLOBAL(uint32_t, 0x0082fd88)) {
                    gp4::call_regs(0x0053d999, c);
                    GP4_FIELD(uint8_t, c.edi, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, c.edi, 0x1f) | 0x40u);
                    GP4_FIELD(uint8_t, c.esi, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, c.esi, 0x1f) | 0x40u);
                    c.ecx = GP4_GLOBAL(uint32_t, 0x0082fd88);
                    GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(c.ecx);
                }
            }
        }

        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edx, 5u));
        if (c.edx == 5u) {
            const uint32_t itemFlags = GP4_FIELD(uint8_t, c.esi, 0x1e) & 0x80u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(itemFlags));
            if (itemFlags != 0u) {
                GP4_GLOBAL(uint8_t, 0x0081e4fa) = 0xffu;
            }
        }

        const uint32_t savedEax = (c.eax & 0xffffff00u) | GP4_FIELD(uint8_t, c.esi, 0x1f);
        c.eax = savedEax;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edx, 2u));
        if (c.edx == 2u) {
            const uint16_t word = static_cast<uint16_t>(GP4_FIELD(uint16_t, c.esi, 0x1f) | 0x80u);
            GP4_FIELD(uint16_t, c.esi, 0x1f) = word;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic16(word));
        }

        const uint32_t savedEbx = GP4_FIELD(uint32_t, c.esi, 0xe);
        const uint32_t savedEsi = c.esi;
        c.ebx = savedEbx;
        gp4::call_regs(GP4_FIELD(uint32_t, c.esi, 0x1a), c);
        c.esi = savedEsi;
        c.ebx = savedEbx;

        gp4::call_regs(0x0053dbbc, c);
        c.eax = savedEax;
        const uint8_t current = GP4_FIELD(uint8_t, c.esi, 0x1f);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(savedEax & 0xffu, current));
        if ((savedEax & 0xffu) != current) {
            const uint8_t marked = static_cast<uint8_t>(current | 0x40u);
            GP4_FIELD(uint8_t, c.esi, 0x1f) = marked;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(marked));
        }
        gp4::call_regs(0x0053dc57, c);
    }

    const uint8_t cleared = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0082fd6a) & 0xfdu);
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = cleared;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x0053daae, FUN_0053daae, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+state_0053d9a6+items_0053dc57+sel_0053d8c5+sel_0053daae+script_all_00835846")
