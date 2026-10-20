#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sister of 0x52bb48 for the setting byte [0x81d610] (copy [0x81d635], changed flag [0x81d61c]).
// When the setting differs from its copy the copy is updated and, if 0x52bd00 reports the setting
// as used (CF), the changed flag is set. Then the 33rd node of the list [0x84d8ec] gets the word
// +0x30 = 0 when the setting is used, else 2 (clearing the changed flag); and when the changed flag
// is clear, the three nodes after it get bit 0x20 of their byte +0x1f set.
// eax keeps its upper bytes with al = the setting, edx = 0 or 2 (dx), ecx = 0 (the 0x21-hop loop
// counts it down to 0, and the three-node loop, when it runs, ends at 0 too), esi = the last node
// reached; the flags are those of the last flag writer.

static inline uint32_t LoadLink(uint32_t addr) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(addr));
}

// FUNCTION: GP4 0x0052bbae
void __cdecl FUN_0052bbae(gp4::Regs* r) {
    const uint32_t setting = GP4_GLOBAL(uint8_t, 0x0081d610);
    uint32_t flags = gp4::flags::sub8(setting, GP4_GLOBAL(uint8_t, 0x0081d635));
    if (setting != GP4_GLOBAL(uint8_t, 0x0081d635)) {
        GP4_GLOBAL(uint8_t, 0x0081d635) = static_cast<uint8_t>(setting);
        gp4::Regs c = *r;
        c.eax = (r->eax & 0xffffff00u) | setting;
        gp4::call_regs(0x0052bd00, c);
        flags = c.eflags;
        if ((c.eflags & gp4::CF) != 0u) {
            GP4_GLOBAL(uint8_t, 0x0081d61c) = 1;
        }
    }

    uint32_t node = 0x0084d8ecu;
    uint32_t count = 0x21u;
    do {
        node = LoadLink(node);
        --count;
    } while (count != 0u);

    const uint32_t current = GP4_GLOBAL(uint8_t, 0x0081d610);
    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | current;
    c.ecx = 0u;
    c.edx = (r->edx & 0xffff0000u);
    c.esi = node;
    gp4::call_regs(0x0052bd00, c);
    flags = c.eflags;
    uint32_t edx = r->edx & 0xffff0000u;
    if ((c.eflags & gp4::CF) == 0u) {
        edx |= 2u;
        GP4_GLOBAL(uint8_t, 0x0081d61c) = 0;
    }
    *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(node + 0x30u)) = static_cast<uint16_t>(edx);

    flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0081d61c), 0u);
    if (GP4_GLOBAL(uint8_t, 0x0081d61c) == 0u) {
        uint32_t ecx = 3u;
        do {
            node = LoadLink(node);
            uint8_t& flags1f = *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(node + 0x1fu));
            flags1f = static_cast<uint8_t>(flags1f | 0x20u);
            flags = gp4::flags::logic8(flags1f);
            flags = gp4::flags::dec32(ecx, flags);
            --ecx;
        } while (ecx != 0u);
    }

    r->eax = (r->eax & 0xffffff00u) | current;
    r->ecx = 0u;
    r->edx = edx;
    r->esi = node;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052bbae, FUN_0052bbae, "regs(eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=list_0084d8ec+set_0081d60f+tab_0081dc63")
