#include <gp4/gp4.h>

// Tail wrapper: `call 0x004023e8 ; ret`. Registers and flags come straight back from the callee.
// FUNCTION: GP4 0x004195db
void __cdecl FUN_004195db(gp4::Regs* r) {
    gp4::call_regs(0x004023e8, *r);
}
GP4_IMPL(0x004195db, FUN_004195db, "regs(esi:ptr[0x210]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
