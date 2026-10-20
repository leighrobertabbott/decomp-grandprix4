#include <gp4/gp4.h>
#include <gp4/flags.h>

// A menu item: next pointer first, a signed word at +6 (the item's y position), flag bytes at +0x1e/+0x1f.
struct Unknown_MenuItem {
    uint32_t Next;
    uint8_t Unknown04[2];
    int16_t Unknown06;
    uint8_t Unknown08[0x16];
    uint8_t Flags1E;
    uint8_t Flags1F;
};

static inline Unknown_MenuItem* menuItem(uint32_t address) {
    return reinterpret_cast<Unknown_MenuItem*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0053d81f
// Walks the item chain of the current menu [0x0082fd34] and picks the item with the largest word +6
// (starting from 0) among those with flag 0x40 in +0x1e and none of 0x30 in +0x1f (eax = its 1-based
// position, 0 = none; bx = that word, edx = the number of links followed). With a pick: edi = the
// picked item, esi = the item at [0x0082fd45] links (the old selection); both get flag 0x40 in +0x1f
// and [0x0082fd45] becomes the pick's position. Without one the registers are left as the walk ended
// (esi = 0) and the flags are those of `or eax, eax`; with one they are those of the last
// `or byte [edi+0x1f], 0x40`.
void __cdecl FUN_0053d81f(gp4::Regs* r) {
    uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t hops = 0u;
    uint32_t pick = 0u;
    int16_t highest = 0;
    for (;;) {
        hops += 1u;
        node = menuItem(node)->Next;
        if (node == 0u) {
            break;
        }
        const Unknown_MenuItem* item = menuItem(node);
        if ((item->Flags1E & 0x40u) != 0u && (item->Flags1F & 0x30u) == 0u && highest < item->Unknown06) {
            highest = item->Unknown06;
            pick = hops;
        }
    }

    r->edx = hops;
    r->ebx = (r->ebx & 0xffff0000u) | static_cast<uint16_t>(highest);
    r->eax = pick;
    r->esi = node;
    if (pick == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }

    gp4::Regs c = {};
    c.ecx = pick;
    gp4::call_regs(0x0053d999, c);
    const uint32_t pickedItem = c.edi;

    const uint32_t selection = GP4_GLOBAL(uint8_t, 0x0082fd45);
    c = {};
    c.ecx = selection;
    gp4::call_regs(0x0053d98c, c);
    const uint32_t oldItem = c.esi;

    menuItem(oldItem)->Flags1F |= 0x40u;
    uint8_t flags1F = menuItem(pickedItem)->Flags1F | 0x40u;
    menuItem(pickedItem)->Flags1F = flags1F;
    GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(pick);

    r->ecx = selection;
    r->esi = oldItem;
    r->edi = pickedItem;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flags1F));
}
GP4_IMPL(0x0053d81f, FUN_0053d81f, "regs(ebx:u32, ecx:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053d879+state_0053d9a6")
