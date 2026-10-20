#include <gp4/gp4.h>

// `call 0x0054ba7a ; ret`: tail wrapper. The callee clears the byte 0x0082fffc, runs 0x0054b686
// (loads ebx/ecx, dl = 0x1d, al = 0 and calls 0x0054b10c over the table 0x0066d8be..0x0066d8c6) and,
// when that leaves CF clear, 0x0054b6a0; eax and edx keep their upper bits from the caller, so both
// are inputs; registers and flags come back as the callee leaves them.
// FUNCTION: GP4 0x005324f6
void __cdecl FUN_005324f6(gp4::Regs* r) {
    gp4::call_regs(0x0054ba7a, *r);
}
GP4_IMPL(0x005324f6, FUN_005324f6, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
