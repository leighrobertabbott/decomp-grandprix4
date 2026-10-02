// Orientation helpers from the hand-written x87 core (0x401000-0x441000 region).
#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00401d80
// Takes a pointer in EDX to a block holding three angles as doubles at +0x10,
// +0x18, +0x20 and stores (sin, cos) pairs for each at +0x28/+0x30, +0x38/+0x40,
// +0x48/+0x50. Building block of the rotation-matrix setup that follows it.
void __cdecl Orient_ComputeAngleSinCos(gp4::Regs* r) {
    uint8_t* o = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));
    for (int i = 0; i < 3; ++i) {
        double s, c;
        gp4::x87::fsincos(GP4_FIELD(double, o, 0x10 + 8 * i), &s, &c);
        GP4_FIELD(double, o, 0x28 + 16 * i) = s;
        GP4_FIELD(double, o, 0x30 + 16 * i) = c;
    }
}
GP4_IMPL(0x00401d80, Orient_ComputeAngleSinCos, "regs(edx:ptr[0x60]:f64) -> ()")
