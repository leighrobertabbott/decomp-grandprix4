#include <gp4/gp4.h>
#include <gp4/flags.h>

// Variable-index read of the menu key list pointers (word table of two list addresses at 0x82fe0a).
static __declspec(noinline) uint32_t keyListFor(uint32_t index) {
    return GP4_ARRAY(uint32_t, 0x0082fe0a)[index];
}

// Menu key dispatch. When bit 0 of the pending flag [0x82fd6a] is set it is cleared, and the masked key
// word ax = [0x7ab00e] & 0x86ff is looked up: first in the list of the current menu's key table (selected by
// byte +0x12 of the menu header [0x82fd34], 1 or 2, each list is a count followed by 6-byte entries of a key
// word and a handler address), then in the 13-entry default table at 0x82fdbc. The matching entry's handler
// is called with ebx = the entry, ecx = the entries left, esi = the item the selection [0x82fd45] points at
// and ax = the key.
// FUNCTION: GP4 0x0053d58a
void __cdecl FUN_0053d58a(gp4::Regs* r) {
    uint8_t pending = GP4_GLOBAL(uint8_t, 0x0082fd6a);
    uint32_t flags = gp4::flags::logic8(pending & 1u);
    if ((pending & 1u) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }
    pending = static_cast<uint8_t>(pending & 0xfeu);
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = pending;
    flags = gp4::flags::logic8(pending);

    uint32_t ax = GP4_GLOBAL(uint16_t, 0x007ab00e) & 0x86ffu;
    flags = gp4::flags::logic16(ax);

    uint32_t esi = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t ecx = GP4_GLOBAL(uint8_t, 0x0082fd45);
    do {
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
        flags = gp4::flags::dec32(ecx, flags);
        ecx -= 1u;
    } while (ecx != 0u);

    uint32_t ebx = GP4_GLOBAL(uint32_t, 0x0082fd34);
    ebx = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ebx) + 0x12u);
    flags = gp4::flags::logic32(ebx);
    bool found = false;
    if (ebx != 0u) {
        flags = gp4::flags::sub32(ebx, 3u);
        if (ebx < 3u) {
            flags = gp4::flags::dec32(ebx, flags);
            ebx -= 1u;
            ebx = keyListFor(ebx);
            ecx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(ebx));
            flags = gp4::flags::add32(ebx, 4u);
            ebx += 4u;
            flags = gp4::flags::logic32(ecx);
            if (ecx != 0u) {
                for (;;) {
                    const uint16_t key = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(ebx));
                    flags = gp4::flags::sub16(ax, key);
                    if (key == static_cast<uint16_t>(ax)) {
                        found = true;
                        break;
                    }
                    flags = gp4::flags::add32(ebx, 6u);
                    ebx += 6u;
                    flags = gp4::flags::dec32(ecx, flags);
                    ecx -= 1u;
                    if (ecx == 0u) {
                        break;
                    }
                }
            }
        }
    }
    if (!found) {
        ebx = 0x0082fdbcu;
        ecx = 0xdu;
        for (;;) {
            const uint16_t key = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(ebx));
            flags = gp4::flags::sub16(ax, key);
            if (key == static_cast<uint16_t>(ax)) {
                found = true;
                break;
            }
            flags = gp4::flags::add32(ebx, 6u);
            ebx += 6u;
            flags = gp4::flags::dec32(ecx, flags);
            ecx -= 1u;
            if (ecx == 0u) {
                break;
            }
        }
    }

    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffff0000u) | ax;
    c.ebx = ebx;
    c.ecx = ecx;
    c.esi = esi;
    c.eflags = gp4::flags::merge(r->eflags, flags);
    c.st_in = 0;
    c.st_out = 0;
    if (found) {
        gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(ebx) + 2u), c);
    }
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0053d58a, FUN_0053d58a, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+state_0053d9a6+sel_0053d879+key_0053d58a")
