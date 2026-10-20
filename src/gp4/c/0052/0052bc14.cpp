#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item node of the list whose head cell is 0x0084d8ec: link first, a flag byte at +0x1f and a
// word at +0x30 (the only fields the menu walkers touch).
struct MenuNode {
    MenuNode* Next00;
    uint8_t Unknown04[0x1b];
    uint8_t Flags1f;
    uint8_t Unknown20[0x10];
    uint16_t Word30;
};

// Walks the menu list, then flags the results of the option byte [0x0081d611]:
// - when the byte differs from its last-seen copy [0x0081d636], the copy is updated and, if table
//   entry 0x0081dc63[byte] is used (!= 0xff, CF from 0x0052bd00), [0x0081d61e] is set to 1;
// - the node 0x26 hops from the head gets word +0x30 = 0 when the entry is used, else 2 (and
//   [0x0081d61e] = 0);
// - when [0x0081d61e] ends up zero, the next three nodes get bit 0x20 set in their flag byte.
// al = the option byte, ecx = 0, dx = the stored word, esi = the last node reached. The flags are
// those of `cmp byte [0x81d61e], 0` (nonzero) or of the last `dec ecx` (CF kept from that compare).
// FUNCTION: GP4 0x0052bc14
void __cdecl FUN_0052bc14(gp4::Regs* r) {
    const uint32_t high = r->eax & 0xffffff00u;
    uint8_t option = GP4_GLOBAL(uint8_t, 0x0081d611);
    if (option != GP4_GLOBAL(uint8_t, 0x0081d636)) {
        GP4_GLOBAL(uint8_t, 0x0081d636) = option;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = high | option;
        gp4::call_regs(0x0052bd00, c);
        if ((c.eflags & gp4::flags::CF) != 0u) {
            GP4_GLOBAL(uint8_t, 0x0081d61e) = 1;
        }
    }

    MenuNode* node = GP4_ARRAY(MenuNode, 0x0084d8ec);
    uint32_t hops = 0x26u;
    do {
        node = node->Next00;
        --hops;
    } while (hops != 0u);

    option = GP4_GLOBAL(uint8_t, 0x0081d611);
    uint32_t word = 0u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = high | option;
    gp4::call_regs(0x0052bd00, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        word = 2u;
        GP4_GLOBAL(uint8_t, 0x0081d61e) = 0;
    }
    node->Word30 = static_cast<uint16_t>(word);

    const uint8_t changed = GP4_GLOBAL(uint8_t, 0x0081d61e);
    uint32_t flags = gp4::flags::sub8(changed, 0u);
    if (changed == 0u) {
        hops = 3u;
        do {
            node = node->Next00;
            node->Flags1f |= 0x20u;
            --hops;
        } while (hops != 0u);
        flags = gp4::flags::dec32(1u, flags);
    }

    r->eax = high | option;
    r->ecx = 0u;
    r->edx = (r->edx & 0xffff0000u) | word;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(node));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052bc14, FUN_0052bc14, "regs(eax:u32, edx:u32) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=list_0084d8ec+set_0081d60f+tab_0081dc63")
