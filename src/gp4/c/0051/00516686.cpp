#include <gp4/gp4.h>

// pushal wrapper: copies the dword at +0x20 of the selected car record [0x00679d30] to
// 0x00a6987c. Every register is restored by popal and no flag is written.
// FUNCTION: GP4 0x00516686
void __cdecl FUN_00516686(gp4::Regs* /*r*/) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_FIELD(uint32_t, car, 0x20);
}
GP4_IMPL(0x00516686, FUN_00516686, "regs() -> (); globals=layout=carbg_0066fef0+state_0044334d")
