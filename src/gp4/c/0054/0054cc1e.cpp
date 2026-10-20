#include <gp4/gp4.h>

// Loads the (unaligned) dword at 0x00833cc5 into ebx and the dword at 0x00833cc9 into eax.
// FUNCTION: GP4 0x0054cc1e
void __cdecl FUN_0054cc1e(gp4::Regs* r) {
    r->ebx = GP4_GLOBAL(uint32_t, 0x00833cc5);
    r->eax = GP4_GLOBAL(uint32_t, 0x00833cc9);
}
GP4_IMPL(0x0054cc1e, FUN_0054cc1e, "regs() -> (eax, ebx); globals=fuzz:i32")
