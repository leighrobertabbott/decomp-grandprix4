#include <gp4/gp4.h>

// Tail wrapper: `call 0x005146b3 ; ret`. The callee (pushal/popal) preserves every register; the flags
// are those of its `test byte [0x66d7a4], 0xff` or of the last flag writer of its drawing call.
// FUNCTION: GP4 0x0051f293
void __cdecl FUN_0051f293(gp4::Regs* r) {
    gp4::call_regs(0x005146b3, *r);
}
GP4_IMPL(0x0051f293, FUN_0051f293, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
