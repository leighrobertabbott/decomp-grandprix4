#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the item mask ebx for the multiplayer slot walk. ebx = 4 and, when the player id [0x00b1bb40] equals
// the host id (0x00541a0a: ZF), the number of used slots (0x005419ea) below 2 adds 0x100; otherwise ebx = 0x8e.
// Then eax = the slot of the id [0x00b1bb40] (0x00541d06) is tested with `or eax, eax` and the tail 0x00540188
// (which keeps its flags as input) walks the static item chain with the final mask. Registers and flags
// come back from it.
// FUNCTION: GP4 0x00540159
void __cdecl FUN_00540159(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = 4u;
    gp4::call_regs(0x00541a0a, c);
    if ((c.eflags & gp4::flags::ZF) != 0u) {
        gp4::call_regs(0x005419ea, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 2u));
        if (c.eax < 2u) {
            c.ebx = c.ebx | 0x100u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.ebx));
        }
    } else {
        c.ebx = 0x8eu;
    }
    c.eax = GP4_GLOBAL(uint32_t, 0x00b1bb40);
    gp4::call_regs(0x00541d06, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    gp4::call_regs(0x00540188, c);
    *r = c;
}
GP4_IMPL(0x00540159, FUN_00540159, "regs() -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+kind_00541cdb+slotsel_00540884+host_00b1bb40+flags_0084869c")
