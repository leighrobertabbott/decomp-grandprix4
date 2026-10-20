#include <gp4/gp4.h>

// Copies the dword at 0x00838e50 to the (unaligned) global at 0x00833cc5; eax keeps the value.
// FUNCTION: GP4 0x0054d2d2
void __cdecl FUN_0054d2d2(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00838e50);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = value;
    r->eax = value;
}
GP4_IMPL(0x0054d2d2, FUN_0054d2d2, "regs() -> (eax); globals=fuzz:i32")
