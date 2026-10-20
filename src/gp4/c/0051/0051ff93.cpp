#include <gp4/gp4.h>

// Copies the dword at 0x00891504 to the global at 0x00679d20; eax keeps the value.
// FUNCTION: GP4 0x0051ff93
void __cdecl FUN_0051ff93(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00891504);
    GP4_GLOBAL(uint32_t, 0x00679d20) = value;
    r->eax = value;
}
GP4_IMPL(0x0051ff93, FUN_0051ff93, "regs() -> (eax); globals=fuzz:i32")
