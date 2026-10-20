#include <gp4/gp4.h>

// Stores the target value 57000 (0xdea8) in [0x852da8] and runs 0x005210d0, which scans the table rows
// at [0x7a8e18] for the one whose value (+4) is closest to it and, when that row's type is 3, resets
// the draw state. It is a pushal/popal routine ending in clc/stc, so every register comes back
// unchanged and CF reports whether a row was selected; the other flags are the last writer's.
// FUNCTION: GP4 0x005210c0
void __cdecl FUN_005210c0(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00852da8) = 0xdea8u;
    gp4::call_regs(0x005210d0, *r);
}
GP4_IMPL(0x005210c0, FUN_005210c0, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584")
