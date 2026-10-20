#include <gp4/gp4.h>

// Loads three dwords (0x00638dd4, 0x00638dd0, 0x00638dd8) into the block at 0x00622cf8, then runs
// the angle conversion (0x00401d5c), the sin/cos pairs (0x00401d80) and the rotation matrix
// (0x00401da2) on that block. edx = block, eax = the last dword loaded.
// FUNCTION: GP4 0x00401fb4
void __cdecl FUN_00401fb4(gp4::Regs* r) {
    uint32_t* block = GP4_ARRAY(uint32_t, 0x00622cf8);

    block[0] = GP4_GLOBAL(uint32_t, 0x00638dd4);
    block[1] = GP4_GLOBAL(uint32_t, 0x00638dd0);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00638dd8);
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    state.edx = 0x00622cf8u;
    gp4::call_regs(0x00401d5c, state);
    gp4::call_regs(0x00401d80, state);
    gp4::call_regs(0x00401da2, state);

    r->eax = last;
    r->edx = 0x00622cf8u;
}
GP4_IMPL(0x00401fb4, FUN_00401fb4, "regs() -> (eax, edx); globals=fuzz:i32")
