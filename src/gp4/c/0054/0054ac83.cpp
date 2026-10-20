#include <gp4/gp4.h>

// `mov al, 3 ; call 0x0054ac8b ; ret`: tail wrapper that latches code 3 (the low byte of eax is
// replaced, the rest of eax is kept; dl takes the latched byte, so edx is an input too). Registers
// and flags come back as the callee leaves them.
// FUNCTION: GP4 0x0054ac83
void __cdecl FUN_0054ac83(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = (r->eax & 0xffffff00u) | 3u;
    gp4::call_regs(0x0054ac8b, c);
    *r = c;
}
GP4_IMPL(0x0054ac83, FUN_0054ac83, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0054b978")
