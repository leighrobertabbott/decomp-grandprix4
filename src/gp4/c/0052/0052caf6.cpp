#include <gp4/gp4.h>

// `mov eax, 4 ; call 0x0052cb10 ; ret`: runs the wait loop of 0x0052cb10 with a counter of 4. The
// callee preserves every register (pushal/popal), so eax comes back as 4; the flags are those of its
// last `cmp word [0x007a9470], 0`.
// FUNCTION: GP4 0x0052caf6
void __cdecl FUN_0052caf6(gp4::Regs* r) {
    r->eax = 4u;
    gp4::call_regs(0x0052cb10, *r);
}
GP4_IMPL(0x0052caf6, FUN_0052caf6, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
