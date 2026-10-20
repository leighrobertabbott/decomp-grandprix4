#include <gp4/gp4.h>

// FUNCTION: GP4 0x00532c59
// Tail wrapper: `call 0x005ae990 ; ret` (the callee is a bare `ret`).
void __cdecl FUN_00532c59(gp4::Regs* r) {
    gp4::call_regs(0x005ae990, *r);
}
GP4_IMPL(0x00532c59, FUN_00532c59, "regs() -> ()")
