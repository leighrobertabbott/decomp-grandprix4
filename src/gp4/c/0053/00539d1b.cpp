#include <gp4/gp4.h>

// Returns edx = 0 (`mov edx, 0; ret`); no flags are touched.
// FUNCTION: GP4 0x00539d1b
void __cdecl FUN_00539d1b(gp4::Regs* r) {
    r->edx = 0u;
}
GP4_IMPL(0x00539d1b, FUN_00539d1b, "regs() -> (edx)")
