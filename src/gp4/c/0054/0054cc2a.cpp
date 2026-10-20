#include <gp4/gp4.h>

// mov ebx, [0x00833cc5] ; ret: loads the (unaligned) dword at 0x00833cc5 into ebx.
// FUNCTION: GP4 0x0054cc2a
void __cdecl FUN_0054cc2a(gp4::Regs* r) {
    r->ebx = GP4_GLOBAL(uint32_t, 0x00833cc5);
}
GP4_IMPL(0x0054cc2a, FUN_0054cc2a, "regs() -> (ebx); globals=fuzz:i32")
