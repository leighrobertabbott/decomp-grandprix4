#include <gp4/gp4.h>

// pushal wrapper: copies the dword 0x00a6987c to 0x00a69880. The esi/eax loads are undone by popal;
// no flag is written.
// FUNCTION: GP4 0x0051fb12
void __cdecl FUN_0051fb12(gp4::Regs* /*r*/) {
    GP4_GLOBAL(uint32_t, 0x00a69880) = GP4_GLOBAL(uint32_t, 0x00a6987c);
}
GP4_IMPL(0x0051fb12, FUN_0051fb12, "regs() -> (); globals=fuzz:i32")
