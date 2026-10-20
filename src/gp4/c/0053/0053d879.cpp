#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu selection search (forward twin of 0x0053d8c5). [0x82fd45] is the current item number (1-based)
// and [0x82fd52] the item count. Starting after the current item and wrapping from the count back
// to 1, each other item is fetched through 0x0053d999 (ecx = item number -> edi = the node reached
// by following the +0 links from the menu header [0x82fd34]) until one is selectable: its flag byte
// +0x1f has neither bit 0x10 nor 0x20, the low two bits of the type byte +0xd are both set and
// byte +0x1e has bit 0x40. That item and the node at esi get bit 0x40 set in their +0x1f byte and
// the item number becomes the current one. Returns when the walk comes back round to the current
// item. al holds the last +0xd & 3, ecx the last item number tried, edi the last node fetched; the
// flags are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x0053d879
void __cdecl FUN_0053d879(gp4::Regs* r) {
    const uint32_t node = r->esi;
    uint32_t ecx = GP4_GLOBAL(uint8_t, 0x0082fd45);
    uint32_t flags;
    for (;;) {
        ecx += 1u;
        if (ecx > GP4_GLOBAL(uint32_t, 0x0082fd52)) {
            ecx = 1u;
        }
        const uint8_t current = GP4_GLOBAL(uint8_t, 0x0082fd45);
        flags = gp4::flags::sub8(static_cast<uint8_t>(ecx), current);
        if (static_cast<uint8_t>(ecx) == current) {
            break;
        }
        gp4::Regs c = {};
        c.ecx = ecx;
        gp4::call_regs(0x0053d999, c);
        const uint32_t item = c.edi;
        r->edi = item;

        const uint8_t state = GP4_FIELD(uint8_t, item, 0x1f);
        flags = gp4::flags::logic8(state & 0x10u);
        if ((state & 0x10u) != 0u) {
            continue;
        }
        flags = gp4::flags::logic8(state & 0x20u);
        if ((state & 0x20u) != 0u) {
            continue;
        }
        const uint32_t type = GP4_FIELD(uint8_t, item, 0x0d) & 3u;
        r->eax = (r->eax & 0xffffff00u) | type;
        flags = gp4::flags::sub8(static_cast<uint8_t>(type), 3u);
        if (type != 3u) {
            continue;
        }
        const uint8_t caps = GP4_FIELD(uint8_t, item, 0x1e);
        flags = gp4::flags::logic8(caps & 0x40u);
        if ((caps & 0x40u) == 0u) {
            continue;
        }
        const uint8_t markedNode = static_cast<uint8_t>(GP4_FIELD(uint8_t, node, 0x1f) | 0x40u);
        GP4_FIELD(uint8_t, node, 0x1f) = markedNode;
        flags = gp4::flags::logic8(markedNode);
        const uint8_t markedItem = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x40u);
        GP4_FIELD(uint8_t, item, 0x1f) = markedItem;
        flags = gp4::flags::logic8(markedItem);
        GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(ecx);
        break;
    }
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053d879, FUN_0053d879, "regs(eax:u32, esi:ptr[0x28]:bytes) -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+sel_0053d8c5+sel_0053d879")
