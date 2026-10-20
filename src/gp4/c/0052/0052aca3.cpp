#include <gp4/gp4.h>

// Reads the local clock through 0x0052accd (eax = wSecond, ecx = minute with the hour in bits 8..15,
// edx = the second in bits 8..15) and stores dx at 0x0081d364 and cx at 0x0081d366. Registers and flags are
// what the callee leaves.
// FUNCTION: GP4 0x0052aca3
void __cdecl FUN_0052aca3(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0052accd, c);
    GP4_GLOBAL(uint16_t, 0x0081d364) = static_cast<uint16_t>(c.edx);
    GP4_GLOBAL(uint16_t, 0x0081d366) = static_cast<uint16_t>(c.ecx);
    *r = c;
}
GP4_IMPL(0x0052aca3, FUN_0052aca3, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
