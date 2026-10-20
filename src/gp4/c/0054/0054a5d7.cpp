#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x0054a573 with the other menu pass: `call 0x0054ba6b; mov al, 0; call 0x0054aab4; clc;
// ret`. 0x0054aab4 runs the menu runner 0x0053c8c8 (message pump) with the page byte 0x007ab1a0
// temporarily set to al. Registers and flags are those it leaves, with CF cleared.
// FUNCTION: GP4 0x0054a5d7
void __cdecl FUN_0054a5d7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ba6b, c);
    c.eax = c.eax & 0xffffff00u;
    gp4::call_regs(0x0054aab4, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x0054a5d7, FUN_0054a5d7, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
