#include <gp4/gp4.h>
#include <gp4/flags.h>

// Under pushal/popal: expands three text scripts one after another into the static buffer 0x00823300 with the
// interpreter 0x0054d798 (edi advances past each text): id 0x76e, then id [0x007c2f28] + 0xb90, then id 0xc2.
// Every register is kept; the flags are those the last expansion leaves.
// FUNCTION: GP4 0x00528dd4
void __cdecl FUN_00528dd4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x76eu;
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x00823300)));
    gp4::call_regs(0x0054d798, c);
    c.esi = GP4_GLOBAL(uint32_t, 0x007c2f28) + 0xb90u;
    gp4::call_regs(0x0054d798, c);
    c.esi = 0xc2u;
    gp4::call_regs(0x0054d798, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00528dd4, FUN_00528dd4, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+textsel_00528c98")
