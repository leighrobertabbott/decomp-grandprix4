#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051ae01
// Tail wrapper: `call 0x004080eb ; ret` (answers in CF from the record at esi).
void __cdecl FUN_0051ae01(gp4::Regs* r) {
    gp4::call_regs(0x004080eb, *r);
}
GP4_IMPL(0x0051ae01, FUN_0051ae01, "regs(esi:ptr[0x1d6]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sign_00868582")
