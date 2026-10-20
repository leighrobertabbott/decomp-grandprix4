#include <gp4/gp4.h>

// Calls 0x004083ac (esi = car record, eax = result, flags returned) and returns eax = 0 when it
// reports carry; otherwise eax and the flags are the callee's. The mov leaves the flags untouched.
// FUNCTION: GP4 0x0051ad87
void __cdecl FUN_0051ad87(gp4::Regs* r) {
    gp4::call_regs(0x004083ac, *r);
    if ((r->eflags & 1u) != 0u) {
        r->eax = 0u;
    }
}
GP4_IMPL(0x0051ad87, FUN_0051ad87, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=list_0066fe3c; cases=200")
