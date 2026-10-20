#include <gp4/gp4.h>

// pushal wrapper: zero-extends the word at +0x62 of the selected car record [0x00679d30] into
// 0x00a6987c. Every register is restored by popal and no flag is written (movzx/mov do not touch flags).
// FUNCTION: GP4 0x00516577
void __cdecl FUN_00516577(gp4::Regs* /*r*/) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_FIELD(uint16_t, car, 0x62);
}
GP4_IMPL(0x00516577, FUN_00516577, "regs() -> (); globals=layout=carbg_0066fef0+state_0044334d")
