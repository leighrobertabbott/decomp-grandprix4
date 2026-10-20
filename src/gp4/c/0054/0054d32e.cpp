#include <gp4/gp4.h>

// FUNCTION: GP4 0x0054d32e
// Tail wrapper: `call 0x0054d6f9 ; ret` (ebx = [0x00822244] >> 1 and the flags of that shift
// come straight back from the callee).
void __cdecl FUN_0054d32e(gp4::Regs* r) {
    gp4::call_regs(0x0054d6f9, *r);
}
GP4_IMPL(0x0054d32e, FUN_0054d32e, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=val_00822244")
