#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (CF, PF, AF, ZF, SF, OF).
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00407839
// Two integer vectors (dx, dz) = (DAT_006279b0 - DAT_006279ac, DAT_006279d0 - DAT_006279cc) and
// (DAT_006279b4 - DAT_006279b8, DAT_006279d4 - DAT_006279d8) are turned into a length
// (round(sqrt(dx*dx + dz*dz))) and an angle (round(atan2(dx, dz) * DAT_006f2be4)).
// The shorter of the two lengths is checked against the limit DAT_006279dc: carry set
// (stc) when the longer one is below the limit, otherwise carry clear (clc) and, when
// only the shorter one is below the limit, its angle is replaced by the longer one's.
// The last cmp's other flags escape, eax/edx hold the two lengths (eax possibly the copied angle).
void __cdecl FUN_00407839(gp4::Regs* r) {
    using namespace gp4::x87;
    const int32_t dx1 = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279b0)) - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279ac)));
    const int32_t dz1 = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279d0)) - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279cc)));
    const Ext fx1 = ext_i32(dx1);
    const Ext fz1 = ext_i32(dz1);
    GP4_GLOBAL(int32_t, 0x00627980) = fistp32(fsqrt(fx1 * fx1 + fz1 * fz1));
    GP4_GLOBAL(int32_t, 0x00627970) = fistp32(fpatan(fx1, fz1) * ext(GP4_GLOBAL(double, 0x006f2be4)));

    const int32_t dx2 = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279b4)) - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279b8)));
    const int32_t dz2 = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279d4)) - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279d8)));
    // the original routes every operand through the scratch dword; its last value stays there
    GP4_GLOBAL(int32_t, 0x006f2bf4) = dz2;
    const Ext fx2 = ext_i32(dx2);
    const Ext fz2 = ext_i32(dz2);
    GP4_GLOBAL(int32_t, 0x00627984) = fistp32(fsqrt(fx2 * fx2 + fz2 * fz2));
    GP4_GLOBAL(int32_t, 0x00627974) = fistp32(fpatan(fx2, fz2) * ext(GP4_GLOBAL(double, 0x006f2be4)));

    const int32_t first = GP4_GLOBAL(int32_t, 0x00627980);
    const int32_t second = GP4_GLOBAL(int32_t, 0x00627984);
    uint32_t eax = static_cast<uint32_t>(first);
    uint32_t flags;
    uint32_t carry;
    if (first < second) {
        const int32_t limit = GP4_GLOBAL(int32_t, 0x006279dc);
        if (second < limit) {
            flags = compareFlags32(static_cast<uint32_t>(second), static_cast<uint32_t>(limit));
            carry = 1u;
        } else {
            flags = compareFlags32(static_cast<uint32_t>(first), static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279dc)));
            carry = 0u;
            if (first < GP4_GLOBAL(int32_t, 0x006279dc)) {
                eax = GP4_GLOBAL(uint32_t, 0x00627974);
                GP4_GLOBAL(uint32_t, 0x00627970) = eax;
            }
        }
    } else {
        const int32_t limit = GP4_GLOBAL(int32_t, 0x006279dc);
        if (first < limit) {
            flags = compareFlags32(static_cast<uint32_t>(first), static_cast<uint32_t>(limit));
            carry = 1u;
        } else {
            flags = compareFlags32(static_cast<uint32_t>(second), static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006279dc)));
            carry = 0u;
            if (second < GP4_GLOBAL(int32_t, 0x006279dc)) {
                eax = GP4_GLOBAL(uint32_t, 0x00627970);
                GP4_GLOBAL(uint32_t, 0x00627974) = eax;
            }
        }
    }
    r->eax = eax;
    r->edx = static_cast<uint32_t>(second);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x00407839, FUN_00407839, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
