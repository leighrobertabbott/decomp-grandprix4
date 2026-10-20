#include <gp4/gp4.h>

// pushal; stores the ten dwords 0x00874dd8..0x00874dfc (four zeros at +8..+0x14, 0x200 twice at
// +0/+4, 0x40 twice at +0x18/+0x1c, zeros at +0x20/+0x24); popal restores every register. The flags
// are untouched (only movs in between).
// FUNCTION: GP4 0x00555b85
void __cdecl FUN_00555b85(gp4::Regs* r) {
    (void)r;
    GP4_GLOBAL(uint32_t, 0x00874de0) = 0;
    GP4_GLOBAL(uint32_t, 0x00874de4) = 0;
    GP4_GLOBAL(uint32_t, 0x00874de8) = 0;
    GP4_GLOBAL(uint32_t, 0x00874dec) = 0;
    GP4_GLOBAL(uint32_t, 0x00874dd8) = 0x200;
    GP4_GLOBAL(uint32_t, 0x00874ddc) = 0x200;
    GP4_GLOBAL(uint32_t, 0x00874df0) = 0x40;
    GP4_GLOBAL(uint32_t, 0x00874df4) = 0x40;
    GP4_GLOBAL(uint32_t, 0x00874df8) = 0;
    GP4_GLOBAL(uint32_t, 0x00874dfc) = 0;
}
GP4_IMPL(0x00555b85, FUN_00555b85, "regs() -> (); globals=fuzz:i32")
