#include <gp4/gp4.h>

// Empty routine: a single `ret`; eax is left exactly as the caller passed it (callers read it).
// FUNCTION: GP4 0x005ae990
void __cdecl FUN_005ae990(gp4::Regs* r) { (void)r; }
GP4_IMPL(0x005ae990, FUN_005ae990, "regs(eax:u32) -> (eax)")
