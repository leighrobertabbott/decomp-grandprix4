#include <gp4/gp4.h>
#include <gp4/x87.h>

// cmp left, right (32-bit) arithmetic flags: CF PF AF ZF SF OF.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00407a0d
// Distance and fraction along a segment. With (A, B) = (DAT_006279a4 - DAT_0062799c,
// DAT_006279a8 - DAT_006279a0), L = DAT_0062797c = round(sqrt(A*A + B*B)):
//   L <  DAT_006279dc: DAT_00627950 = DAT_006f2a54 (double)
//   otherwise          DAT_00627950 = clamp(round((DAT_006278c0 - DAT_0062799c) * sin(t) +
//                                                  (DAT_006278c8 - DAT_006279a0) * cos(t)), 0, L) / L
//                      with t = atan2(A, B).
// DAT_006f2bf4 is the fild/fistp scratch dword. eax/edx/ebp are left as the routine leaves them.
void __cdecl FUN_00407a0d(gp4::Regs* r) {
    using namespace gp4::x87;
    int32_t& scratch = GP4_GLOBAL(int32_t, 0x006f2bf4);

    uint32_t eax = GP4_GLOBAL(uint32_t, 0x006279a4) - GP4_GLOBAL(uint32_t, 0x0062799c);
    scratch = static_cast<int32_t>(eax);
    const Ext a = ext_i32(scratch);
    const Ext aa = a * a;
    uint32_t edx = GP4_GLOBAL(uint32_t, 0x006279a8) - GP4_GLOBAL(uint32_t, 0x006279a0);
    scratch = static_cast<int32_t>(edx);
    const Ext b = ext_i32(scratch);
    const Ext bb = b * b;
    GP4_GLOBAL(int32_t, 0x0062797c) = fistp32(fsqrt(aa + bb));
    const int32_t length = GP4_GLOBAL(int32_t, 0x0062797c);       // ebp
    const uint32_t flagsLimit = subtractionFlags32(static_cast<uint32_t>(length), GP4_GLOBAL(uint32_t, 0x006279dc));

    uint32_t flags = flagsLimit;
    if (length < GP4_GLOBAL(int32_t, 0x006279dc)) {
        GP4_GLOBAL(double, 0x00627950) = narrow(ext(GP4_GLOBAL(double, 0x006f2a54)));
    } else {
        scratch = static_cast<int32_t>(eax);
        const Ext y = ext_i32(scratch);
        scratch = static_cast<int32_t>(edx);
        const Ext x = ext_i32(scratch);
        const Ext angle = fpatan(y, x);
        Ext sine;
        Ext cosine;
        fsincos(angle, &sine, &cosine);

        eax = GP4_GLOBAL(uint32_t, 0x006278c0) - GP4_GLOBAL(uint32_t, 0x0062799c);
        edx = GP4_GLOBAL(uint32_t, 0x006278c8) - GP4_GLOBAL(uint32_t, 0x006279a0);
        scratch = static_cast<int32_t>(edx);
        const Ext fromEdx = cosine * ext_i32(scratch);
        scratch = static_cast<int32_t>(eax);
        const Ext fromEax = sine * ext_i32(scratch);
        scratch = fistp32(fromEdx + fromEax);

        int32_t projected = scratch;
        if (projected < 0) {
            projected = 0;
        }
        flags = subtractionFlags32(static_cast<uint32_t>(projected), static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062797c)));
        if (projected > GP4_GLOBAL(int32_t, 0x0062797c)) {
            projected = GP4_GLOBAL(int32_t, 0x0062797c);
        }
        scratch = projected;
        const Ext ratio = ext_i32(scratch) / ext_i32(GP4_GLOBAL(int32_t, 0x0062797c));
        GP4_GLOBAL(double, 0x00627950) = narrow(ratio);
        eax = static_cast<uint32_t>(projected);
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = static_cast<uint32_t>(length);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00407a0d, FUN_00407a0d, "regs() -> (eax,edx,ebp,cf,pf,af,zf,sf,of); globals=fuzz:i32")
