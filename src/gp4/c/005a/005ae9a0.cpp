#include <gp4/gp4.h>

// Sets al to 1 (`mov al, 1; ret`); the upper 24 bits of eax are untouched and callers read them.
// FUNCTION: GP4 0x005ae9a0
void __cdecl FUN_005ae9a0(gp4::Regs* r) {
    r->eax = (r->eax & 0xffffff00u) | 1u;
}
GP4_IMPL(0x005ae9a0, FUN_005ae9a0, "regs(eax:u32) -> (eax)")
