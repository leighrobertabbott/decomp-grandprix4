#include <gp4/gp4.h>

// FUNCTION: GP4 0x004319d9
void __cdecl FUN_004319d9(gp4::Regs* r) {
    r->eflags &= ~1u;
}
GP4_IMPL(0x004319d9, FUN_004319d9, "regs() -> (cf)")
