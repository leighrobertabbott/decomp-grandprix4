#include <gp4/gp4.h>

// Loads three dwords (0x00638034, 0x00638030, 0x00638038) into the block at 0x00622e98, then runs
// the angle conversion (0x00401d5c), the sin/cos pairs (0x00401d80) and the rotation matrix
// (0x00401da2) on that block. edx = block, eax = the last dword loaded.
// FUNCTION: GP4 0x00401fe0
void __cdecl FUN_00401fe0(gp4::Regs* r) {
    uint32_t* block = GP4_ARRAY(uint32_t, 0x00622e98);

    block[0] = GP4_GLOBAL(uint32_t, 0x00638034);
    block[1] = GP4_GLOBAL(uint32_t, 0x00638030);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00638038);
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    state.edx = 0x00622e98u;
    gp4::call_regs(0x00401d5c, state);
    gp4::call_regs(0x00401d80, state);
    gp4::call_regs(0x00401da2, state);

    r->eax = last;
    r->edx = 0x00622e98u;
}
GP4_IMPL(0x00401fe0, FUN_00401fe0, "regs() -> (eax, edx); globals=fuzz:i32")
