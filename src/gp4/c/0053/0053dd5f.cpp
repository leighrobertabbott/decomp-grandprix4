#include <gp4/gp4.h>
#include <gp4/flags.h>

// A menu item: next pointer first, an i16 rectangle (left, top, right, bottom) at +4..+0xa, the item id
// byte at +0xc, flag bytes at +0x1e/+0x1f.
struct Unknown_MenuHitItem {
    uint32_t Next;
    int16_t Unknown04;
    int16_t Unknown06;
    int16_t Unknown08;
    int16_t Unknown0A;
    uint8_t Unknown0C;
    uint8_t Unknown0D[0x11];
    uint8_t Flags1E;
    uint8_t Flags1F;
};

static inline Unknown_MenuHitItem* hitItem(uint32_t address) {
    return reinterpret_cast<Unknown_MenuHitItem*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0053dd5f
// Menu hit test: walks the item chain of the current menu [0x0082fd34] (no null test of the header
// itself) for the first item that is not hidden (+0x1f & 0x18), whose signed word rectangle contains
// the point (bx = x, ax = y) and, when [0x0082fd44] is 0xff, whose id byte +0xc equals [0x0082fd40].
// esi = the item (0 when the chain ends), edx = its id, CF = 0 for a usable hit. An item with flag
// 0x10 or 0x20 in +0x1f, or the end of the chain, gives CF = 1 and edx = 0. eax and ebx are preserved.
// Flags: `or esi, esi` (chain end: ZF = PF = 1) or `test byte [esi+0x1f], mask`, then clc/stc.
void __cdecl FUN_0053dd5f(gp4::Regs* r) {
    const int16_t x = static_cast<int16_t>(r->ebx);
    const int16_t y = static_cast<int16_t>(r->eax);
    uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t id = 0u;
    uint32_t flags = 0u;
    bool hit = false;
    for (;;) {
        node = hitItem(node)->Next;
        if (node == 0u) {
            flags = gp4::flags::logic32(0u);
            break;
        }
        const Unknown_MenuHitItem* item = hitItem(node);
        if ((item->Flags1F & 0x18u) != 0u || x < item->Unknown04 || y < item->Unknown06 ||
            x > item->Unknown08 || y > item->Unknown0A) {
            continue;
        }
        const uint32_t itemId = item->Unknown0C;
        if (GP4_GLOBAL(uint8_t, 0x0082fd44) == 0xffu && itemId != GP4_GLOBAL(uint32_t, 0x0082fd40)) {
            continue;
        }
        id = itemId;
        if ((item->Flags1F & 0x10u) != 0u) {
            flags = gp4::flags::logic8(item->Flags1F & 0x10u);
            break;
        }
        flags = gp4::flags::logic8(item->Flags1F & 0x20u);
        if ((item->Flags1F & 0x20u) != 0u) {
            break;
        }
        hit = true;
        break;
    }
    if (hit) {
        flags &= ~gp4::flags::CF;       // clc
        r->edx = id;
    } else {
        flags |= gp4::flags::CF;        // stc
        r->edx = 0u;
    }
    r->esi = node;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053dd5f, FUN_0053dd5f, "regs(eax:i32[190..445], ebx:u32{0,480,487,488,489,495,500,503,504,510,0x123401f0}) -> (edx, esi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+state_0053d9a6")
