#include <gp4/gp4.h>

// Sets the byte 0x007a9790 to 0xff and copies the dword at 0x007a9784 to 0x007a9780; eax keeps the
// copied value. The flags are not touched.
// FUNCTION: GP4 0x00541f02
void __cdecl FUN_00541f02(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007a9790) = 0xffu;
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007a9784);
    GP4_GLOBAL(uint32_t, 0x007a9780) = value;
    r->eax = value;
}
GP4_IMPL(0x00541f02, FUN_00541f02, "regs() -> (eax); globals=fuzz:i32")
