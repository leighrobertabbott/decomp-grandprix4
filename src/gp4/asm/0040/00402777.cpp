#include <gp4/gp4.h>
#include <gp4/x87.h>

// Reads the double at [edx + address].
static inline gp4::x87::Ext loadQ(uint32_t base, uint32_t address) {
    return gp4::x87::ext(*reinterpret_cast<const double*>(static_cast<uintptr_t>(address + base)));
}

// Four doubles q0..q3 (at DAT_00622c00 + edx, stride 8; a quaternion) and the scale DAT_006f2a64 give three
// angles, all kept in extended precision until the qword stores:
//   DAT_00622ca0 = atan2(K * (q1*q3 - q0*q2), (q0^2 - q1^2) - (q2^2 - q3^2))
//   DAT_00622ca8 = atan2(K * (q0*q3 - q1*q2), (q0^2 - q1^2) + (q2^2 - q3^2))
//   DAT_00622c98 = atan2(s, sqrt(1 - s^2))   with s = K * (q0*q1 + q2*q3); the radicand is replaced by 0
//                  unless it is strictly positive (an unordered compare keeps it).
// The final `fucomp; fnstsw ax; sahf` leaves CF/PF/ZF from the compare of 0 with the radicand, AF = 1 (TOP = 6
// with two values on the stack), SF = 0, and the status word in ax. The oracle (Unicorn/QEMU x87) keeps no sticky
// precision-exception bit in the status word, so ax = TOP (6) | C3/C2/C0 of the compare; real hardware would also
// show PE (0x20) there, but every caller overwrites eax right after the call.
// FUNCTION: GP4 0x00402777
void __cdecl FUN_00402777(gp4::Regs* r) {
    using namespace gp4::x87;
    const uint32_t offset = r->edx;
    const Ext q0 = loadQ(offset, 0x00622c00);
    const Ext q1 = loadQ(offset, 0x00622c08);
    const Ext q2 = loadQ(offset, 0x00622c10);
    const Ext q3 = loadQ(offset, 0x00622c18);
    const Ext scale = ext(GP4_GLOBAL(double, 0x006f2a64));

    const Ext a = q0 * q0 - q1 * q1;
    const Ext b = q2 * q2 - q3 * q3;
    const Ext sum = a + b;
    const Ext difference = a - b;

    const Ext c = q1 * q3 - q0 * q2;
    GP4_GLOBAL(double, 0x00622ca0) = narrow(fpatan(c * scale, difference));

    const Ext d = q0 * q3 - q1 * q2;
    GP4_GLOBAL(double, 0x00622ca8) = narrow(fpatan(d * scale, sum));

    const Ext s = (q0 * q1 + q2 * q3) * scale;
    const Ext radicand = fchs(s * s) + ext_one();
    const uint16_t compare = fucom(ext_zero(), radicand);
    Ext root = ext_zero();
    if ((compare & 0x0100u) != 0u) {
        root = fsqrt(radicand);
    } else {
        root = fsqrt(ext_zero());
    }
    GP4_GLOBAL(double, 0x00622c98) = narrow(fpatan(s, root));

    uint32_t flags = 0x10u;
    if ((compare & 0x0100u) != 0u) flags |= 0x01u;
    if ((compare & 0x0400u) != 0u) flags |= 0x04u;
    if ((compare & 0x4000u) != 0u) flags |= 0x40u;
    r->eax = (r->eax & 0xffff0000u) | compare | 0x3000u;
    r->eflags = (r->eflags & ~0xd5u) | flags;
}
GP4_IMPL(0x00402777, FUN_00402777, "regs(eax:u32, edx:u32[0..0]) -> (eax, cf, pf, af, zf, sf); globals=fuzz:f64")
