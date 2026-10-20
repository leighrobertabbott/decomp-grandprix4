#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: expands the text scripts 0xb25, [0x007c2f28] + 0xb90 and 0xc2 into the static
// buffer 0x00823300 with the interpreter 0x0054d798 (esi = script id, edi = output). popal restores
// every register; the flags are those the last interpreter run leaves.
// FUNCTION: GP4 0x00528cc5
void __cdecl FUN_00528cc5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0xb25u;
    c.edi = 0x00823300u;
    gp4::call_regs(0x0054d798, c);
    const uint32_t selector = GP4_GLOBAL(uint32_t, 0x007c2f28);
    c.esi = selector + 0xb90u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(selector, 0xb90u));
    gp4::call_regs(0x0054d798, c);
    c.esi = 0xc2u;
    gp4::call_regs(0x0054d798, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00528cc5, FUN_00528cc5, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+textsel_00528c98")
