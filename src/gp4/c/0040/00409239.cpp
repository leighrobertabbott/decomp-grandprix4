#include <gp4/gp4.h>

// FUNCTION: GP4 0x00409239
// `call 0x004041dd ; call 0x00409244 ; ret`: both calls work on the esi record and the edi
// parameter block; registers and flags come straight back from the second call.
void __cdecl FUN_00409239(gp4::Regs* r) {
    gp4::call_regs(0x004041dd, *r);
    gp4::call_regs(0x00409244, *r);
}
GP4_IMPL(0x00409239, FUN_00409239, "regs(edi:ptr[0x6a]:bytes, esi:ptr[0x150]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
