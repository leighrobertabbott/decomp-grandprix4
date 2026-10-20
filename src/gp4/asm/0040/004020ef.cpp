#include <gp4/gp4.h>
#include <gp4/x87.h>

// Row-by-row product of two 3x3 double matrices stored at +0x58..+0x98 of three 0xa0-byte blocks:
// ecx[i][j] = edx row i . eax row j (each cell is a dot product of three consecutive doubles,
// 0x18 bytes per row). Each product and the running sum stay on the x87 stack at 80 bits; only the
// final sum is stored as a double.
static inline gp4::x87::Ext Dot3(uint8_t* a, uint8_t* b) {
    using namespace gp4::x87;
    const Ext p0 = ext(GP4_FIELD(double, a, 0x00)) * ext(GP4_FIELD(double, b, 0x00));
    const Ext p1 = ext(GP4_FIELD(double, a, 0x08)) * ext(GP4_FIELD(double, b, 0x08));
    const Ext p2 = ext(GP4_FIELD(double, a, 0x10)) * ext(GP4_FIELD(double, b, 0x10));
    return (p0 + p1) + p2;
}

// FUNCTION: GP4 0x004020ef
void __cdecl FUN_004020ef(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* rhs = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ecx));
    uint8_t* lhs = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    GP4_FIELD(double, dst, 0x58) = narrow(Dot3(lhs + 0x58, rhs + 0x58));
    GP4_FIELD(double, dst, 0x60) = narrow(Dot3(lhs + 0x58, rhs + 0x70));
    GP4_FIELD(double, dst, 0x68) = narrow(Dot3(lhs + 0x58, rhs + 0x88));
    GP4_FIELD(double, dst, 0x70) = narrow(Dot3(lhs + 0x70, rhs + 0x58));
    GP4_FIELD(double, dst, 0x78) = narrow(Dot3(lhs + 0x70, rhs + 0x70));
    GP4_FIELD(double, dst, 0x80) = narrow(Dot3(lhs + 0x70, rhs + 0x88));
    GP4_FIELD(double, dst, 0x88) = narrow(Dot3(lhs + 0x88, rhs + 0x58));
    GP4_FIELD(double, dst, 0x90) = narrow(Dot3(lhs + 0x88, rhs + 0x70));
    GP4_FIELD(double, dst, 0x98) = narrow(Dot3(lhs + 0x88, rhs + 0x88));
}
GP4_IMPL(0x004020ef, FUN_004020ef, "regs(eax:ptr[0xa0]:f64, ecx:ptr[0xa0]:f64, edx:ptr[0xa0]:f64) -> ()")
