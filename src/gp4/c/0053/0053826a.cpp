#include <gp4/gp4.h>

// Clears the flag byte 0x00829ba3 (0x0053827a), then opens the screen record 0x00829bc0 with 0x0053c8c8
// (esi = the record). Registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x0053826a
void __cdecl FUN_0053826a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0053827a, c);
    c.esi = 0x00829bc0u;
    gp4::call_regs(0x0053c8c8, c);
    *r = c;
}
GP4_IMPL(0x0053826a, FUN_0053826a, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846")
