#include <gp4/gp4.h>

// Loads esi from the (unaligned) dword at 0x0082ff0f (`mov esi, [0x82ff0f]; ret`); flags untouched.
// FUNCTION: GP4 0x0053ee49
void __cdecl FUN_0053ee49(gp4::Regs* r) {
    r->esi = GP4_GLOBAL(uint32_t, 0x0082ff0f);
}
GP4_IMPL(0x0053ee49, FUN_0053ee49, "regs() -> (esi); globals=fuzz:i32")
