#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// FUNCTION: GP4 0x004239dc
// If |DAT_00607f54| >= DAT_006080e0 then DAT_006080d4 = DAT_006080dc and eax is
// adjusted by DAT_006080d0: subtracted when the sign bit of byte DAT_0060827d is
// set, added otherwise. Otherwise eax is untouched. edx leaves holding |DAT_00607f54|
// (or DAT_006080dc) and the flags are those of the last cmp / add / sub.
void __cdecl FUN_004239dc(gp4::Regs* r) {
    int32_t edx = GP4_GLOBAL(int32_t, 0x00607f54);
    if (edx < 0) {
        edx = (int32_t)(0u - (uint32_t)edx);
    }
    const int32_t limit = GP4_GLOBAL(int32_t, 0x006080e0);
    uint32_t fl = r->eflags & ~0x8D5u;
    if (edx >= limit) {
        edx = GP4_GLOBAL(int32_t, 0x006080dc);
        GP4_GLOBAL(int32_t, 0x006080d4) = edx;
        const uint32_t a = r->eax;
        const uint32_t b = GP4_GLOBAL(uint32_t, 0x006080d0);
        uint32_t res;
        if (GP4_GLOBAL(int8_t, 0x0060827d) < 0) {
            res = a - b;
            if (a < b) fl |= 0x001u;
            if (((a ^ b) & (a ^ res)) & 0x80000000u) fl |= 0x800u;
        } else {
            res = a + b;
            if (res < a) fl |= 0x001u;
            if (((a ^ res) & (b ^ res)) & 0x80000000u) fl |= 0x800u;
        }
        if ((a ^ b ^ res) & 0x10u) fl |= 0x010u;
        fl |= parityFlag(res);
        if (res == 0) fl |= 0x040u;
        if (res & 0x80000000u) fl |= 0x080u;
        r->eax = res;
    } else {
        // flags of: cmp edx, limit
        const uint32_t a = (uint32_t)edx;
        const uint32_t b = (uint32_t)limit;
        const uint32_t res = a - b;
        if (a < b) fl |= 0x001u;
        if (((a ^ b) & (a ^ res)) & 0x80000000u) fl |= 0x800u;
        if ((a ^ b ^ res) & 0x10u) fl |= 0x010u;
        fl |= parityFlag(res);
        if (res == 0) fl |= 0x040u;
        if (res & 0x80000000u) fl |= 0x080u;
    }
    r->edx = (uint32_t)edx;
    r->eflags = fl;
}
GP4_IMPL(0x004239dc, FUN_004239dc, "regs(eax:i32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
