#include <gp4/gp4.h>

// Three register routines in a row, each handing its registers to the next: 0x0054cb7e, 0x0054d1ad
// (esi = a string-table index, replaced by the string address) and 0x0054d211 (pushal-wrapped call of
// the debug-log writer 0x004fd820; it preserves every register). Registers and flags come back from the
// last call.
// FUNCTION: GP4 0x0053ce63
void __cdecl FUN_0053ce63(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054cb7e, c);
    gp4::call_regs(0x0054d1ad, c);
    gp4::call_regs(0x0054d211, c);
    *r = c;
}
GP4_IMPL(0x0053ce63, FUN_0053ce63, "regs(eax:u32, ebx:u32, esi:u32[0..0x3ff]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
