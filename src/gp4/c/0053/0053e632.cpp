#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper over the linked list that starts at esi. 0x0053e939 collects the flagged nodes into the
// 0x20-byte entry table at 0x00822300 (count in [0x0082fefb]); for each entry (edi = the entry) 0x0053e6d0
// finds its nearest neighbours and leaves the four neighbour ids in [0x0082fecb], [0x0082fed7],
// [0x0082fee3] and [0x0082feef]. Each id other than 0xf9 replaces the node's matching byte (+0x22, +0x23,
// +0x24, +0x25) when that byte still holds the 0xf9 "none" marker. popal restores every register, then bit 1
// of the head node's byte +0x11 is cleared (`and byte [esi + 0x11], 0xfd`), which also sets the flags.
// FUNCTION: GP4 0x0053e632
void __cdecl FUN_0053e632(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0053e939, c);

    c.edi = 0x00822300u;
    if (GP4_GLOBAL(uint32_t, 0x0082fefb) != 0u) {
        c.ecx = 0u;
        do {
            gp4::call_regs(0x0053e6d0, c);
            const uint32_t node = GP4_FIELD(uint32_t, c.edi, 0);
            c.esi = node;
            if (GP4_GLOBAL(uint32_t, 0x0082fecb) != 0xf9u && GP4_FIELD(uint8_t, node, 0x22) == 0xf9u) {
                GP4_FIELD(uint8_t, node, 0x22) = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x0082fecb));
            }
            if (GP4_GLOBAL(uint32_t, 0x0082fed7) != 0xf9u && GP4_FIELD(uint8_t, node, 0x23) == 0xf9u) {
                GP4_FIELD(uint8_t, node, 0x23) = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x0082fed7));
            }
            if (GP4_GLOBAL(uint32_t, 0x0082fee3) != 0xf9u && GP4_FIELD(uint8_t, node, 0x24) == 0xf9u) {
                GP4_FIELD(uint8_t, node, 0x24) = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x0082fee3));
            }
            if (GP4_GLOBAL(uint32_t, 0x0082feef) != 0xf9u && GP4_FIELD(uint8_t, node, 0x25) == 0xf9u) {
                GP4_FIELD(uint8_t, node, 0x25) = static_cast<uint8_t>(GP4_GLOBAL(uint32_t, 0x0082feef));
            }
            c.edi += 0x20u;
            c.ecx += 1u;
        } while (c.ecx != GP4_GLOBAL(uint32_t, 0x0082fefb));
    }

    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->esi, 0x11) & 0xfdu);
    GP4_FIELD(uint8_t, r->esi, 0x11) = cleared;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x0053e632, FUN_0053e632, "regs(eax:u32, edx:u32, esi:ptr[0x28]:layout=lnode1_0053e612) -> (cf, pf, af, zf, sf, of)")
