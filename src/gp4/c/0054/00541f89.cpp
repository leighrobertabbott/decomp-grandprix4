#include <gp4/gp4.h>

// Runs the debug-fill routine 0x00537f02 (registers and flags are whatever it leaves), then clears
// [0x00830bac] and sets the dwords [0x008312cc] and [0x008312d0] to -1.
// FUNCTION: GP4 0x00541f89
void __cdecl FUN_00541f89(gp4::Regs* r) {
    gp4::call_regs(0x00537f02, *r);
    GP4_GLOBAL(uint32_t, 0x00830bac) = 0u;
    GP4_GLOBAL(uint32_t, 0x008312cc) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x008312d0) = 0xffffffffu;
}
GP4_IMPL(0x00541f89, FUN_00541f89, "regs() -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=dlog_00537f02")
