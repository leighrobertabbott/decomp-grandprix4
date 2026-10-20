#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs 0x00531af4 with eax = 0 (it stores 0 in 0x00827559 and restores every register), loads edi from
// [0x00838c14], then sets the byte 0x00838c29 = 0xff and 0x00838c28 = 0. The flags are those the
// callee leaves.
// FUNCTION: GP4 0x0054dd4a
void __cdecl FUN_0054dd4a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0u;
    gp4::call_regs(0x00531af4, c);
    r->eax = c.eax;
    r->edi = GP4_GLOBAL(uint32_t, 0x00838c14);
    GP4_GLOBAL(uint8_t, 0x00838c29) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00838c28) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054dd4a, FUN_0054dd4a, "regs() -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
