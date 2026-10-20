#include <gp4/gp4.h>

// `call 0x0053646a; ret`: tail wrapper, the closer of the open file handle runs with the entry
// registers and leaves its registers and flags.
// FUNCTION: GP4 0x00538264
void __cdecl FUN_00538264(gp4::Regs* r) {
    gp4::call_regs(0x0053646a, *r);
}
GP4_IMPL(0x00538264, FUN_00538264, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
