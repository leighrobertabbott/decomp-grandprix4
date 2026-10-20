#include <gp4/gp4.h>

// `mov eax, 9 ; call 0x0052cb10 ; ret`: runs the wait loop of 0x0052cb10 with a counter of 9. The
// callee preserves every register (pushal/popal), so eax comes back as 9; the flags are those of its
// last `cmp word [0x007a9470], 0`.
// FUNCTION: GP4 0x0052caeb
void __cdecl FUN_0052caeb(gp4::Regs* r) {
    r->eax = 9u;
    gp4::call_regs(0x0052cb10, *r);
}
GP4_IMPL(0x0052caeb, FUN_0052caeb, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
