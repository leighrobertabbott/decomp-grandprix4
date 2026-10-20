#include <gp4/gp4.h>

// Copies two dwords out of the selected car record [0x00679d30]: +0x70 -> 0x00a69880 and
// +0x28 -> 0x00a6987c. pushal/popal restore every register; the movs leave the flags untouched.
// FUNCTION: GP4 0x00516792
void __cdecl FUN_00516792(gp4::Regs* r) {
    (void)r;
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00679d30);
    GP4_GLOBAL(uint32_t, 0x00a69880) = GP4_FIELD(uint32_t, record, 0x70);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = GP4_FIELD(uint32_t, record, 0x28);
}
GP4_IMPL(0x00516792, FUN_00516792, "regs() -> (); globals=layout=ptr_00679d30")
