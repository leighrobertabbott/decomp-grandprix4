#include <gp4/gp4.h>

// Calls 0x004083f2 (esi = selector, eax = result, flags returned) and returns eax = 0 when it
// reports carry; otherwise eax and the flags are the callee's. The mov leaves the flags untouched.
// FUNCTION: GP4 0x0051ada1
void __cdecl FUN_0051ada1(gp4::Regs* r) {
    gp4::call_regs(0x004083f2, *r);
    if ((r->eflags & 1u) != 0u) {
        r->eax = 0u;
    }
}
GP4_IMPL(0x0051ada1, FUN_0051ada1, "regs(esi:u32[6750988..6750988]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=list_004083f2")
