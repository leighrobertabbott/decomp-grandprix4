#include <gp4/gp4.h>

// Leaves the previous mode: runs the state reset 0x0040faab, clears the byte 0x006f2c28, runs 0x0051e4c5,
// sets the byte 0x0095404c to 0x80 and runs the sound dispatcher 0x005188bc. When the byte 0x007c2ea9 is
// 0x80 it also runs 0x0052046a, then 0x00510efb, and finally clears the byte 0x0088f635. One register
// block is threaded through the callees (they are hand-written and pass eax / edx along); the flags are
// those the last callee leaves.
// FUNCTION: GP4 0x0052353b
void __cdecl FUN_0052353b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0040faab, c);
    GP4_GLOBAL(uint8_t, 0x006f2c28) = 0u;
    gp4::call_regs(0x0051e4c5, c);
    GP4_GLOBAL(uint8_t, 0x0095404c) = 0x80u;
    gp4::call_regs(0x005188bc, c);
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        gp4::call_regs(0x0052046a, c);
    }
    gp4::call_regs(0x00510efb, c);
    GP4_GLOBAL(uint8_t, 0x0088f635) = 0u;
    *r = c;
}
GP4_IMPL(0x0052353b, FUN_0052353b, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+carx_0042fe2f+cur_00852d30+state_0040faab+cf_0041a2b2+ffb_0051e380+carbg_0066fef0+state_0040e84e+carno_0066ff6c+cars_0066fef0+lap_0043b482+state_0044334d+state_0040dfe9+pick_00443148+rank_005202a4; gfields={0x007c2ea9:u8{0x80,0x80,0,1}}")
