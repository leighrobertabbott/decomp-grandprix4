#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040dfe9
// `call 0x0051f1d4 ; jae skip ; call 0x00408f7b ; skip: ret`: the first routine answers in CF
// (set = at least two of the 22 records at 0x007c2a48 are flagged while 0x007ad8b2 == 0xff);
// when CF is set the second routine marks every active record. Registers are preserved by
// both (clc/stc and pushal/popal), the flags come straight back from the last call.
void __cdecl FUN_0040dfe9(gp4::Regs* r) {
    gp4::call_regs(0x0051f1d4, *r);
    if ((r->eflags & 1u) != 0u) {
        gp4::call_regs(0x00408f7b, *r);
    }
}
GP4_IMPL(0x0040dfe9, FUN_0040dfe9, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040dfe9+flags_00408f7b")
