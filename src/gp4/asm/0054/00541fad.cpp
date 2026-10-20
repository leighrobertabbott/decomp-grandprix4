#include <gp4/gp4.h>

// pushal wrapper: sets the [0x006b35f0] bytes starting at the pointer [0x006975cc] to 1 (rep stosb with
// al = 1, direction flag clear). popal restores every GPR; rep stosb writes no flag.
// FUNCTION: GP4 0x00541fad
void __cdecl FUN_00541fad(gp4::Regs* /*r*/) {
    uint8_t* cursor = GP4_GLOBAL(uint8_t*, 0x006975cc);
    uint32_t count = GP4_GLOBAL(uint32_t, 0x006b35f0);
    while (count != 0u) {
        *cursor = 1u;
        ++cursor;
        --count;
    }
}
GP4_IMPL(0x00541fad, FUN_00541fad, "regs() -> (); globals=fuzz:i32+layout=fill_00541fad")
