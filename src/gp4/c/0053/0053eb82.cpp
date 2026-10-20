#include <gp4/gp4.h>
#include <gp4/flags.h>

// A menu item: next pointer first, flag bytes at +0x1e/+0x1f.
struct Unknown_MenuItem {
    uint32_t Next;
    uint8_t Unknown04[0x1a];
    uint8_t Flags1E;
    uint8_t Flags1F;
};

static inline Unknown_MenuItem* menuItem(uint32_t address) {
    return reinterpret_cast<Unknown_MenuItem*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0053eb82
// Mouse-over selection of a menu item. Runs once per pending event: needs [0x007ab180] != 0,
// [0x0082fd88] == 0 and the pending byte [0x007a9468] != 0, which it clears. Hit-tests the menu
// at the pointer position (ebx = [0x0082fd94], eax = [0x0082fd98]) with 0x0053dd5f; a usable hit
// (esi = the item, edx = its id) that has flag 0x40 in +0x1e, none of 0x38 in +0x1f and is not
// already the selection [0x0082fd45] becomes the selection: the old selection's item (edi, found
// by following the chain ecx = old selection links) and the new one get flag 0x40 in +0x1f and
// [0x0082fd45] = the new id. Registers are left as each exit found them; the flags are those of
// the last compare, test or `or` executed.
void __cdecl FUN_0053eb82(gp4::Regs* r) {
    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x007ab180);
    if (enabled == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u));
        return;
    }
    const uint32_t busy = GP4_GLOBAL(uint32_t, 0x0082fd88);
    if (busy != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(busy, 0u));
        return;
    }
    const uint32_t pending = GP4_GLOBAL(uint8_t, 0x007a9468);
    if (pending == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(pending, 0u));
        return;
    }
    GP4_GLOBAL(uint8_t, 0x007a9468) = 0u;

    r->ebx = GP4_GLOBAL(uint32_t, 0x0082fd94);
    r->eax = GP4_GLOBAL(uint32_t, 0x0082fd98);

    gp4::Regs hit = {};
    hit.eax = r->eax;
    hit.ebx = r->ebx;
    hit.eflags = r->eflags;
    gp4::call_regs(0x0053dd5f, hit);
    r->edx = hit.edx;
    r->esi = hit.esi;
    r->eflags = gp4::flags::merge(r->eflags, hit.eflags);
    if ((hit.eflags & gp4::flags::CF) != 0u) {
        return;
    }

    const Unknown_MenuItem* item = menuItem(hit.esi);
    const uint32_t flags1E = item->Flags1E & 0x40u;
    if (flags1E == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flags1E));
        return;
    }
    const uint32_t flags1F = item->Flags1F & 0x38u;
    if (flags1F != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flags1F));
        return;
    }
    const uint32_t selection = GP4_GLOBAL(uint8_t, 0x0082fd45);
    r->ecx = selection;
    if (selection == hit.edx) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(selection, hit.edx));
        return;
    }

    gp4::Regs chain = {};
    chain.ecx = selection;
    gp4::call_regs(0x0053d999, chain);
    r->edi = chain.edi;

    menuItem(hit.esi)->Flags1F |= 0x40u;
    const uint8_t last = static_cast<uint8_t>(menuItem(chain.edi)->Flags1F | 0x40u);
    menuItem(chain.edi)->Flags1F = last;
    GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(hit.edx);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(last));
}
GP4_IMPL(0x0053eb82, FUN_0053eb82, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+sel_0053d879+state_0053d9a6")
