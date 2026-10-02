#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040ccfe
void __cdecl FUN_0040ccfe(gp4::Regs* r) {
    r->eflags |= 1u;
}
GP4_IMPL(0x0040ccfe, FUN_0040ccfe, "regs() -> (cf)")
