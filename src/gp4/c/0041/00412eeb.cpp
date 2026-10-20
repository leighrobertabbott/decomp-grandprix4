#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00412eeb
// angle = atan2(DAT_00601334 * sin(a1 * K), |DAT_00601354 * cos(a2 * K) + (DAT_00601acc << 7)|) * M
// where a1 = (int16)(DAT_00601c60 + DAT_00639642) and a2 = (int16)DAT_00601c60, K = DAT_006f29cc,
// M = DAT_006f29d4. The rounded angle is left in the scratch dword DAT_006f2bf4; its
// magnitude is limited to DAT_006013ac and the signed result is scaled by DAT_00603504
// (imul, shrd 14) into DAT_006034f0. eax is preserved; edx leaves holding the high half
// of the imul and the flags are those of the final shrd.
void __cdecl FUN_00412eeb(gp4::Regs* r) {
    using namespace gp4::x87;

    GP4_GLOBAL(int32_t, 0x006f2bf4) =
        (int32_t)(int16_t)(uint16_t)(GP4_GLOBAL(uint16_t, 0x00601c60) + GP4_GLOBAL(uint16_t, 0x00639642));
    Ext s = fsin(ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * ext(GP4_GLOBAL(double, 0x006f29cc)));
    s = ext_i32(GP4_GLOBAL(int32_t, 0x00601334)) * s;

    GP4_GLOBAL(int32_t, 0x006f2bf4) = (int32_t)GP4_GLOBAL(int16_t, 0x00601c60);
    Ext c = fcos(ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)) * ext(GP4_GLOBAL(double, 0x006f29cc)));
    c = ext_i32(GP4_GLOBAL(int32_t, 0x00601354)) * c;

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = GP4_GLOBAL(uint32_t, 0x00601acc) << 7;
    const Ext sum = c + ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const Ext angle = fpatan(s, fabs(sum)) * ext(GP4_GLOBAL(double, 0x006f29d4));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(angle);

    uint32_t v = GP4_GLOBAL(uint32_t, 0x006f2bf4);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
    if ((int32_t)v < 0) {
        v = 0u - v;
        if (!(v < limit)) {
            v = limit;
        }
        v = 0u - v;
    } else if (!(v < limit)) {
        v = limit;
    }

    const int64_t product = (int64_t)(int32_t)v * (int64_t)GP4_GLOBAL(int32_t, 0x00603504);
    const uint32_t low = (uint32_t)product;
    const uint32_t high = (uint32_t)((uint64_t)product >> 32);
    const uint32_t result = (uint32_t)((uint64_t)product >> 14);
    const uint32_t previous = (uint32_t)((uint64_t)product >> 13);
    GP4_GLOBAL(uint32_t, 0x006034f0) = result;

    // shrd by 14: CF = last bit shifted out; OF = bit31(previous ^ result), AF = 0 (pinned oracle model)
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00412eeb, FUN_00412eeb, "regs() -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
