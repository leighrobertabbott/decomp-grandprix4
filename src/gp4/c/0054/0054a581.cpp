#include <gp4/gp4.h>
#include <gp4/flags.h>

// `call 0x0054ba6b; mov byte [0x007ab1a0], 0; mov al, 0xff; call 0x0054ab53; clc; ret`: advances the
// 3-bit counter with 0x0054ba6b, clears the byte 0x007ab1a0, then runs the menu pass 0x0054ab53 with
// al = 0xff (it waits for a key through the menu runner and the message pump). Registers and flags are
// those 0x0054ab53 leaves, with CF cleared.
// FUNCTION: GP4 0x0054a581
void __cdecl FUN_0054a581(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ba6b, c);
    GP4_GLOBAL(uint8_t, 0x007ab1a0) = 0u;
    c.eax = (c.eax & 0xffffff00u) | 0xffu;
    gp4::call_regs(0x0054ab53, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x0054a581, FUN_0054a581, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
