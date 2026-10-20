#include <gp4/gp4.h>
#include <gp4/flags.h>

// `call 0x0054ba6b; call 0x0054a3ba; call 0x0054a25d; clc; ret`: advances the 3-bit counter, runs the
// menu page 0x0054a3ba (clears [0x7aafd3], sets [0x7aafd8] = 0xff and runs the menu runner with
// esi = 0x8331ac, which waits through the message pump) and then 0x0054a25d. Registers and flags are
// those 0x0054a25d leaves, with CF cleared.
// FUNCTION: GP4 0x0054a690
void __cdecl FUN_0054a690(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ba6b, c);
    gp4::call_regs(0x0054a3ba, c);
    gp4::call_regs(0x0054a25d, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x0054a690, FUN_0054a690, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
