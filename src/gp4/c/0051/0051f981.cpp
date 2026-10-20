#include <gp4/gp4.h>

// pushal / mov eax, 0 / mov [0xa6987c], eax / popal / ret: clears the global at 0x00a6987c;
// every register and flag is preserved.
// FUNCTION: GP4 0x0051f981
void __cdecl FUN_0051f981(gp4::Regs* /*r*/) {
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
}
GP4_IMPL(0x0051f981, FUN_0051f981, "regs() -> (); globals=fuzz:i32")
