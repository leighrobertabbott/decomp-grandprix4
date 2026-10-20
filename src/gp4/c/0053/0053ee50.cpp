#include <gp4/gp4.h>

// Loads the (unaligned) dword at 0x0082ff0f into edx, then calls the empty routine 0x00531b32.
// Flags are untouched.
// FUNCTION: GP4 0x0053ee50
void __cdecl FUN_0053ee50(gp4::Regs* r) {
    r->edx = GP4_GLOBAL(uint32_t, 0x0082ff0f);
    gp4::cdecl_<0x00531b32, void>();
}
GP4_IMPL(0x0053ee50, FUN_0053ee50, "regs() -> (edx); globals=fuzz:i32")
