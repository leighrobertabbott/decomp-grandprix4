#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00402564
// Quaternion-style product of the two 4-vectors of doubles at 0x00622c20 (a0..a3, stride 8)
// and 0x00622c40 (b0..b3): the four results go to 0x00622c60..0x00622c78. Every term is kept
// on the x87 stack at extended precision and rounded to a double only at the store.
void __cdecl FUN_00402564(gp4::Regs* r) {
    using namespace gp4::x87;
    (void)r;

    const Ext a0 = ext(GP4_GLOBAL(double, 0x00622c20));
    const Ext a1 = ext(GP4_GLOBAL(double, 0x00622c28));
    const Ext a2 = ext(GP4_GLOBAL(double, 0x00622c30));
    const Ext a3 = ext(GP4_GLOBAL(double, 0x00622c38));
    const Ext b0 = ext(GP4_GLOBAL(double, 0x00622c40));
    const Ext b1 = ext(GP4_GLOBAL(double, 0x00622c48));
    const Ext b2 = ext(GP4_GLOBAL(double, 0x00622c50));
    const Ext b3 = ext(GP4_GLOBAL(double, 0x00622c58));

    GP4_GLOBAL(double, 0x00622c60) = narrow(((a0 * b0 - a1 * b1) - a2 * b2) - a3 * b3);
    GP4_GLOBAL(double, 0x00622c68) = narrow(((a0 * b1 + a1 * b0) + a2 * b3) - a3 * b2);
    GP4_GLOBAL(double, 0x00622c70) = narrow(((a0 * b2 - a1 * b3) + a2 * b0) + a3 * b1);
    GP4_GLOBAL(double, 0x00622c78) = narrow(((a0 * b3 + a1 * b2) - a2 * b1) + a3 * b0);
}
GP4_IMPL(0x00402564, FUN_00402564, "regs() -> (); globals=fuzz:f64")
