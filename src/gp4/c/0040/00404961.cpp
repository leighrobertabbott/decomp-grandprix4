#include <gp4/gp4.h>

// FUNCTION: GP4 0x00404961
// `call 0x0040489c ; call 0x004048c4 ; ret`: the first call stores the length of the vector
// (+0xa4, +0xa8) at +0x98 (leaving eax/edx/ebp), the second stores the angle of the same vector
// at +0x18. Both read esi; registers and flags come straight back from the calls.
void __cdecl FUN_00404961(gp4::Regs* r) {
    gp4::call_regs(0x0040489c, *r);
    gp4::call_regs(0x004048c4, *r);
}
GP4_IMPL(0x00404961, FUN_00404961, "regs(esi:ptr[0xb0]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of)")
