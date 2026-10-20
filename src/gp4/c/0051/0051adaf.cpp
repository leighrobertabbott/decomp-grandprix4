#include <gp4/gp4.h>

// Returns the constant table address from 0x00418914 in eax and sets ebx = 0x10.
// FUNCTION: GP4 0x0051adaf
void __cdecl FUN_0051adaf(gp4::Regs* r) {
    r->eax = gp4::cdecl_<0x00418914, uint32_t>();
    r->ebx = 0x10u;
}
GP4_IMPL(0x0051adaf, FUN_0051adaf, "regs() -> (eax, ebx)")
