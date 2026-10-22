#include <gp4/gp4.h>

// Start-up of the text/console subsystem: builds the string-pointer table (0x0054cd69), the character-case
// tables (0x0054d70c), then runs the cursor/colour setters with a fixed state: 0x0054cb71 (eax = 0), the
// text cursor x = ebx = 0 and y = eax = 0x40 (0x0054cb7e), the colour 0x7fff (0x0054cc96). Returns eax = -1.
// esi, edi and ebp are pushed and popped; the other registers and the flags thread from callee to callee,
// so ebx = 0, ecx/edx come from the case-table builder and the flags are the colour setter's.
// FUNCTION: GP4 0x0054c8e2
void __cdecl FUN_0054c8e2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0054cd69, c);
    gp4::call_regs(0x0054d70c, c);
    c.eax = 0u;
    gp4::call_regs(0x0054cb71, c);
    c.ebx = 0u;
    c.eax = 0x40u;
    gp4::call_regs(0x0054cb7e, c);
    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);
    c.eax = 0xffffffffu;
    c.esi = r->esi;
    c.edi = r->edi;
    c.ebp = r->ebp;
    *r = c;
}
GP4_IMPL(0x0054c8e2, FUN_0054c8e2, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 CharUpperA")
