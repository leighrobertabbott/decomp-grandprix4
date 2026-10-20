#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keeps one entry of the menu node list [0x84d8ec] in step with the setting byte [0x81d60f]. When the
// setting differs from its copy [0x81d634] the copy is updated and, if 0x52bd00 reports the setting
// as used (CF), the changed flag [0x81d617] is set. Then the 19th node of the list gets the word
// +0x30 = 0 when the setting is used and +0x30 = 2 (clearing [0x81d617]) when it is not, and the six
// nodes after it get bit 0x20 of their byte +0x1f cleared (the `or al` with al = 0 changes nothing).
// eax keeps its upper bytes with al = 0, ecx = 0, esi = the link after the sixth node; the flags are
// those of the last `dec ecx` (CF from the final `or`, i.e. clear).

static inline uint32_t LoadLink(uint32_t addr) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(addr));
}

// FUNCTION: GP4 0x0052bb48
void __cdecl FUN_0052bb48(gp4::Regs* r) {
    const uint32_t setting = GP4_GLOBAL(uint8_t, 0x0081d60f);
    if (setting != GP4_GLOBAL(uint8_t, 0x0081d634)) {
        GP4_GLOBAL(uint8_t, 0x0081d634) = static_cast<uint8_t>(setting);
        gp4::Regs c = *r;
        c.eax = (r->eax & 0xffffff00u) | setting;
        gp4::call_regs(0x0052bd00, c);
        if ((c.eflags & gp4::CF) != 0u) {
            GP4_GLOBAL(uint8_t, 0x0081d617) = 1;
        }
    }

    uint32_t node = 0x0084d8ecu;
    for (int hop = 0; hop < 0x13; ++hop) {
        node = LoadLink(node);
    }
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0081d60f);
    gp4::call_regs(0x0052bd00, c);
    if ((c.eflags & gp4::CF) != 0u) {
        *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(node + 0x30u)) = 0;
    } else {
        GP4_GLOBAL(uint8_t, 0x0081d617) = 0;
        *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(node + 0x30u)) = 2;
    }

    node = LoadLink(node);
    for (int n = 0; n < 6; ++n) {
        uint8_t& flags1f = *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(node + 0x1fu));
        flags1f = static_cast<uint8_t>(flags1f & 0xdfu);
        node = LoadLink(node);
    }

    r->eax = r->eax & 0xffffff00u;
    r->ecx = 0u;
    r->esi = node;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, gp4::flags::logic8(0u)));
}
GP4_IMPL(0x0052bb48, FUN_0052bb48, "regs(eax:u32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=list_0084d8ec+set_0081d60f+tab_0081dc63")
