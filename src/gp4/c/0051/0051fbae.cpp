#include <gp4/gp4.h>

// Returns the constant pair eax = 0x1e, edx = 5; no flags are touched.
// FUNCTION: GP4 0x0051fbae
void __cdecl FUN_0051fbae(gp4::Regs* r) {
    r->eax = 0x1eu;
    r->edx = 5u;
}
GP4_IMPL(0x0051fbae, FUN_0051fbae, "regs() -> (eax, edx)")
