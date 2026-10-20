#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hover state machine for the hotspot list of [0x0082fd34] (pushal ... popal: every register is
// kept, only the flags of the last flag writer escape). Copies the cursor [0x007a9460]/[0x007a9464]
// to [0x00838e40]/[0x00838e44]; when the menu has a hotspot list (+0x1c), hit-tests it with
// 0x0054ede8 and steps the state byte [0x00838e3e] (jump table 0x0054ecdc, states >= 4 use case 3),
// tracking the hovered node [0x00838e48] and the counter [0x00838e74] (the node's byte +0x11), and
// placing the tooltip box with 0x0054ee4f (ebx = the jump-table slot address, as in the original).
// FUNCTION: GP4 0x0054ec8b
void __cdecl FUN_0054ec8b(gp4::Regs* r) {
    const uint32_t cx = GP4_GLOBAL(uint32_t, 0x007a9460);
    const uint32_t cy = GP4_GLOBAL(uint32_t, 0x007a9464);
    GP4_GLOBAL(uint32_t, 0x00838e40) = cx;
    GP4_GLOBAL(uint32_t, 0x00838e44) = cy;

    const uint32_t menu = GP4_GLOBAL(uint32_t, 0x0082fd34);
    if (menu == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(menu));
        return;
    }
    const uint32_t list = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(menu + 0x1cu));
    if (list == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(list, 0u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = cx;
    c.eax = cy;
    c.esi = menu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(list, 0u));
    gp4::call_regs(0x0054ede8, c);

    const uint32_t node = c.eax;
    uint32_t state = GP4_GLOBAL(uint8_t, 0x00838e3e);
    if (state >= 4u) {
        state = 3u;
    }
    c.ebx = (state << 2) + 0x0054ecdcu;

    uint32_t flags = gp4::flags::logic32(node);
    switch (state) {
    case 0u:
        if (node != 0u) {
            GP4_GLOBAL(uint32_t, 0x00838e48) = node;
            GP4_GLOBAL(uint32_t, 0x00838e74) = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node))[0x11];
            GP4_GLOBAL(uint8_t, 0x00838e3e) = 1u;
        }
        break;
    case 1u:
        if (node == 0u) {
            GP4_GLOBAL(uint32_t, 0x00838e48) = 0u;
            GP4_GLOBAL(uint8_t, 0x00838e3e) = 0u;
            break;
        }
        flags = gp4::flags::sub32(node, GP4_GLOBAL(uint32_t, 0x00838e48));
        if (node != GP4_GLOBAL(uint32_t, 0x00838e48)) {
            GP4_GLOBAL(uint32_t, 0x00838e48) = node;
            GP4_GLOBAL(uint32_t, 0x00838e74) = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node))[0x11];
            break;
        }
        flags = gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00838e74), 0u);
        if (GP4_GLOBAL(uint32_t, 0x00838e74) == 0u) {
            c.eflags = gp4::flags::merge(c.eflags, flags);
            gp4::call_regs(0x0054ee4f, c);
            flags = c.eflags;
            GP4_GLOBAL(uint8_t, 0x00838e3e) = 2u;
            break;
        }
        // falls into the case-2 body (0x0054ed66)
        flags = gp4::flags::logic32(node);
        // fallthrough
    case 2u:
        if (node == 0u) {
            GP4_GLOBAL(uint8_t, 0x00838e3e) = 3u;
            GP4_GLOBAL(uint32_t, 0x00838e74) = 9u;
            break;
        }
        flags = gp4::flags::sub32(node, GP4_GLOBAL(uint32_t, 0x00838e48));
        if (node != GP4_GLOBAL(uint32_t, 0x00838e48)) {
            GP4_GLOBAL(uint32_t, 0x00838e48) = node;
            GP4_GLOBAL(uint32_t, 0x00838e74) = 2u;
            GP4_GLOBAL(uint8_t, 0x00838e3e) = 1u;
        }
        break;
    default: {
        if (node == 0u) {
            flags = gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00838e74), 0u);
            if (GP4_GLOBAL(uint32_t, 0x00838e74) == 0u) {
                GP4_GLOBAL(uint8_t, 0x00838e3e) = 0u;
            }
            break;
        }
        const uint8_t bits = static_cast<uint8_t>(reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node))[0x10] & 8u);
        flags = gp4::flags::logic8(bits);
        if (bits != 0u) {
            GP4_GLOBAL(uint32_t, 0x00838e48) = node;
            GP4_GLOBAL(uint32_t, 0x00838e74) = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node))[0x11];
            GP4_GLOBAL(uint8_t, 0x00838e3e) = 1u;
            break;
        }
        GP4_GLOBAL(uint32_t, 0x00838e48) = node;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x0054ee4f, c);
        flags = c.eflags;
        GP4_GLOBAL(uint8_t, 0x00838e3e) = 2u;
        break;
    }
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054ec8b, FUN_0054ec8b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=hot_0054ede8+tip_0054ee4f+pool_00538a24+hover_0054ec8b")
