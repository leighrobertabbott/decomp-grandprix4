#include <gp4/gp4.h>

// Converts the two double constants stored in the code at 0x0051fae6 and 0x0051fade to float, one at a
// time through the scratch float 0x006f2bf4: eax = the bits of (float)[0x51fae6], edx = the bits of
// (float)[0x51fade]. The scratch float is left holding the second value.
// FUNCTION: GP4 0x0051faee
void __cdecl FUN_0051faee(gp4::Regs* r) {
    GP4_GLOBAL(float, 0x006f2bf4) = static_cast<float>(GP4_GLOBAL(double, 0x0051fae6));
    r->eax = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    GP4_GLOBAL(float, 0x006f2bf4) = static_cast<float>(GP4_GLOBAL(double, 0x0051fade));
    r->edx = GP4_GLOBAL(uint32_t, 0x006f2bf4);
}
GP4_IMPL(0x0051faee, FUN_0051faee, "regs() -> (eax, edx)")
