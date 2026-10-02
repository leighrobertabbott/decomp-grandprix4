#include <gp4/gp4.h>

// FUNCTION: GP4 0x00435521
void __cdecl FUN_00435521(gp4::Regs* r) {
    r->edx = GP4_GLOBAL(uint32_t, 0x00622570);
    r->eax = GP4_GLOBAL(uint32_t, 0x00622568);
}
GP4_IMPL(0x00435521, FUN_00435521, "regs() -> (eax, edx); globals=fuzz:i32")
