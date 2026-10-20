#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fourth walker of the 0x52bae5 menu group (with 0x52bb48/0x52bbae/0x52bc14): walks 0x2b links of
// the menu node list [0x84d8ec] and asks 0x52bd00 whether the option byte [0x81d612] is used (CF,
// table entry 0x81dc63[byte] != 0xff). When it is not used, the node reached and the two after it
// get bit 0x20 of their byte +0x1f set (flag first, then advance).
// eax keeps its upper bytes with al = the option byte, ecx = 0, esi = the last node reached; the
// flags are 0x52bd00's when the option is used, else those of the last `dec ecx` (CF from the
// final `or`, i.e. clear).

static inline uint32_t LoadLink(uint32_t addr) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(addr));
}

// FUNCTION: GP4 0x0052bc7a
void __cdecl FUN_0052bc7a(gp4::Regs* r) {
    uint32_t node = 0x0084d8ecu;
    uint32_t count = 0x2bu;
    do {
        node = LoadLink(node);
        --count;
    } while (count != 0u);

    const uint32_t option = GP4_GLOBAL(uint8_t, 0x0081d612);
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | option;
    c.ecx = 0u;
    c.esi = node;
    gp4::call_regs(0x0052bd00, c);
    uint32_t flags = c.eflags;

    if ((c.eflags & gp4::CF) == 0u) {
        uint32_t ecx = 3u;
        do {
            uint8_t& flags1f = *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(node + 0x1fu));
            flags1f = static_cast<uint8_t>(flags1f | 0x20u);
            flags = gp4::flags::logic8(flags1f);
            node = LoadLink(node);
            flags = gp4::flags::dec32(ecx, flags);
            --ecx;
        } while (ecx != 0u);
    }

    r->eax = (r->eax & 0xffffff00u) | option;
    r->ecx = 0u;
    r->esi = node;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052bc7a, FUN_0052bc7a, "regs(eax:u32) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=layout=list_0084d8ec+set_0081d60f+tab_0081dc63")
