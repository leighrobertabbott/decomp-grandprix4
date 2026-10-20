#include <gp4/gp4.h>

// `mov eax, [0x007ad894] ; call 0x00536a48 ; ret`: tail wrapper. eax = the circuit number goes to the
// screen loader 0x00536a48 (it loads the circuit file, then waits through 0x00531f9c). The wait reaches the
// message pump 0x00509b50 (via 0x0053d350 / 0x0054a2c3) and so PeekMessageA. Registers and flags come
// back as the callee leaves them.
// FUNCTION: GP4 0x00532360
void __cdecl FUN_00532360(gp4::Regs* r) {
    r->eax = GP4_GLOBAL(uint32_t, 0x007ad894);
    gp4::call_regs(0x00536a48, *r);
}
GP4_IMPL(0x00532360, FUN_00532360, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
