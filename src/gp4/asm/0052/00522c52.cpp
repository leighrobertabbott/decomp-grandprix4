#include <gp4/gp4.h>

// pushal; copies the 0x16 dwords (0x58 bytes) at 0x00675970 to 0x007c3184 (`rep movsd`, direction flag
// clear); popal restores every register. The flags are untouched.
// FUNCTION: GP4 0x00522c52
void __cdecl FUN_00522c52(gp4::Regs* r) {
    (void)r;
    const uint32_t* src = GP4_ARRAY(uint32_t, 0x00675970);
    uint32_t* dst = GP4_ARRAY(uint32_t, 0x007c3184);
    for (uint32_t i = 0; i != 0x16u; ++i) {
        dst[i] = src[i];
    }
}
GP4_IMPL(0x00522c52, FUN_00522c52, "regs() -> (); globals=fuzz:i32")
