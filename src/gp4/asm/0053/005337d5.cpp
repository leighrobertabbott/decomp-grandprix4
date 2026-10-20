#include <gp4/gp4.h>

// Tail wrapper: passes the value in ebx to 0x00528623 in eax (`mov eax, ebx; call 0x528623; ret`).
// All other registers and the flags come back from the callee.
// FUNCTION: GP4 0x005337d5
void __cdecl FUN_005337d5(gp4::Regs* r) {
    r->eax = r->ebx;
    gp4::call_regs(0x00528623, *r);
}
GP4_IMPL(0x005337d5, FUN_005337d5, "regs(ebx:u32) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_007aafd3")
