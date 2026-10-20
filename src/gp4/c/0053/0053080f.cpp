#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu setup pass: marks the slot records (0x0053090c), loads gconfig.txt (0x00530931) and runs the
// confirmation menu loop (0x0053081f, which waits through the menu runner and the message pump). The
// registers and flags carry from one call to the next; everything comes back from the last one.
// FUNCTION: GP4 0x0053080f
void __cdecl FUN_0053080f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053090c, c);
    gp4::call_regs(0x00530931, c);
    gp4::call_regs(0x0053081f, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053080f, FUN_0053080f, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4; emu=skip:win32 PeekMessageA")
