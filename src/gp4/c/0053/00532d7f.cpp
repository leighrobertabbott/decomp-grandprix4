#include <gp4/gp4.h>

// `call 0x0054b8fe ; ret`: tail wrapper. The callee stores the packet of slot [0x007aafd0] & 7 through
// 0x0054b978 and re-reads it with 0x0054b910 (ax = the slot byte); registers and flags come back as
// the callee leaves them.
// FUNCTION: GP4 0x00532d7f
void __cdecl FUN_00532d7f(gp4::Regs* r) {
    gp4::call_regs(0x0054b8fe, *r);
}
GP4_IMPL(0x00532d7f, FUN_00532d7f, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0054b978")
