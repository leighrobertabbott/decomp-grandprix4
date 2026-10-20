#include <gp4/gp4.h>

// `call 0x005404b6 ; ret`: tail wrapper. The callee polls the network/session object through
// 0x00507360, stores a slot id in [0x007ab023] and a state byte in [0x007ab024]. Registers and flags
// come back as the callee leaves them.
// FUNCTION: GP4 0x005335eb
void __cdecl FUN_005335eb(gp4::Regs* r) {
    gp4::call_regs(0x005404b6, *r);
}
GP4_IMPL(0x005335eb, FUN_005335eb, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
