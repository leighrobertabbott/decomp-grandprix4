#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatches on the byte [0x00831d64] (values above 4 use entry 4) through the five-entry handler
// table at 0x005497cc, calling the selected handler with eax = the entry's address. The handler runs
// with the flags of `add eax, 0x5497cc`; every register and flag comes back from it.
// FUNCTION: GP4 0x005497af
void __cdecl FUN_005497af(gp4::Regs* r) {
    uint32_t index = GP4_GLOBAL(uint8_t, 0x00831d64);
    if (index >= 5u) {
        index = 4u;
    }
    const uint32_t offset = index << 2;
    const uint32_t entry = offset + 0x005497ccu;
    gp4::Regs c = *r;
    c.eax = entry;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(offset, 0x005497ccu));
    gp4::call_regs(GP4_GLOBAL(uint32_t, entry), c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x005497af, FUN_005497af, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_005497af")
