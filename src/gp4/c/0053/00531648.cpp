#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs a registered list of callback pairs. [0x8274ad] is the entry count, the
// entries are 12 bytes each (two code pointers + one more dword) and the list
// is chosen by the byte flag at 0x7ad869 (0x8274b1 when set, else 0x827505).
// For each entry the second routine runs first; when it returns with CF clear
// the first routine runs too. ecx and esi are saved/restored around the loop.

// FUNCTION: GP4 0x00531648
void __cdecl FUN_00531648(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x008274ad);
    uint32_t entry = GP4_GLOBAL(uint8_t, 0x007ad869) ? 0x008274b1u : 0x00827505u;
    gp4::Regs c = *r;
    const uint32_t savedEcx = r->ecx;
    const uint32_t savedEsi = r->esi;
    while (count != 0) {
        c.ecx = count;
        c.esi = entry;
        gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry + 4)), c);
        if (!(c.eflags & gp4::CF)) {
            gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry)), c);
        }
        entry = c.esi + 0xc;
        count = c.ecx - 1;
    }
    *r = c;
    r->ecx = savedEcx;
    r->esi = savedEsi;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0));
}
GP4_IMPL(0x00531648, FUN_00531648, "regs(ebx:ptr[0x40]:bytes) -> (eax, ebx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=cb_00531648")
