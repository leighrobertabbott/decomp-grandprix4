#include <gp4/gp4.h>

// pushal wrapper: stores 0 in 0x00a6987c. The load of esi from [0x00a69878] and `mov eax, 0` are
// undone by popal; no flag is written.
// FUNCTION: GP4 0x0051f89d
void __cdecl FUN_0051f89d(gp4::Regs* /*r*/) {
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
}
GP4_IMPL(0x0051f89d, FUN_0051f89d, "regs() -> (); globals=fuzz:i32")
