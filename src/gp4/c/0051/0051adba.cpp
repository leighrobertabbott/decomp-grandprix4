#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051adba
// Tail wrapper: `call 0x00408351 ; ret` (eax = byte at esi + 0x202).
void __cdecl FUN_0051adba(gp4::Regs* r) {
    gp4::call_regs(0x00408351, *r);
}
GP4_IMPL(0x0051adba, FUN_0051adba, "regs(esi:ptr[0x20a]:bytes) -> (eax)")
