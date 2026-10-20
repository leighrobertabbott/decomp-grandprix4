#include <gp4/gp4.h>

// FUNCTION: GP4 0x005324fc
// Tail wrapper: `call 0x0054ba8e ; ret` (the callee stores 0xff into the byte 0x0082fffc).
void __cdecl FUN_005324fc(gp4::Regs* r) {
    gp4::call_regs(0x0054ba8e, *r);
}
GP4_IMPL(0x005324fc, FUN_005324fc, "regs() -> (); globals=fuzz:i32")
