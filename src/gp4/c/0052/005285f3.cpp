#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; frame-slot maintenance dispatcher; popal; ret.
// When the byte 0x0082fd5e is zero the key-1 slots are cleared through 0x005284d8; otherwise the key-1
// slot lookup 0x005284b3 runs and, when it finds none, the frame object at 0x0081ce7c is registered
// through 0x00528140. popal restores every general register to its entry value but leaves the flags of
// the last callee (or of the `or eax, eax` after the lookup).
// FUNCTION: GP4 0x005285f3
void __cdecl FUN_005285f3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    if (GP4_GLOBAL(uint8_t, 0x0082fd5e) == 0u) {
        c.eax = 1u;
        gp4::call_regs(0x005284d8, c);
    } else {
        c.eax = 1u;
        gp4::call_regs(0x005284b3, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        if (c.eax == 0u) {
            c.eax = 0x0081ce7cu;
            gp4::call_regs(0x00528140, c);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005285f3, FUN_005285f3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_0052848b+frames_008174f8")
