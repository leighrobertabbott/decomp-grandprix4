#include <gp4/gp4.h>

// Copies dword 0x3c of the table at 0x00891bd0 (address 0x00891cc0) to 0x00852d38. eax is pushed and
// popped around the copy and the flags are untouched.
// FUNCTION: GP4 0x0051f09a
void __cdecl FUN_0051f09a(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint32_t, 0x00852d38) = GP4_ARRAY(uint32_t, 0x00891bd0)[0x3c];
}
GP4_IMPL(0x0051f09a, FUN_0051f09a, "regs() -> (); globals=fuzz:i32")
