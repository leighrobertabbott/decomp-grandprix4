#include <gp4/gp4.h>

// FUNCTION: GP4 0x00416bb5
void __cdecl FUN_00416bb5(gp4::Regs* r) {
    r->ebx = reinterpret_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x006380d8));
}
GP4_IMPL(0x00416bb5, FUN_00416bb5, "regs() -> (ebx)")
