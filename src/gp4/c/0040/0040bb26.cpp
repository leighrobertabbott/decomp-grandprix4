#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040bb26
// pushal ... popal routine. After 0x0040baa8 (which sets the clamped factor 0x00622778) the grid
// size n = ((0x0062250c * 0x00622778) >> 17 + 1) >> 1 (at least 1) is stored to 0x006227ac and
// 2n to 0x006227b0; the angle word 0x00622514 selects two Q14 table entries (cos/sin pair),
// giving the step vector esi/edi = -(entry >> 3). After clearing the four accumulators
// 0x006227b4..c0, 0x0040b99a rebuilds its tables and 0x0040b955 walks the four corners
// (ebp = 2n passes). The accumulators are then divided by n << 10 and clamped to 0xb0 (pair at
// 0x0063b8a0) and 0x70 (pair at 0x0063b8a8). Registers are restored by popal; only the flags of
// the final "cmp ecx, 2" (equal: ZF and PF) escape.
void __cdecl FUN_0040bb26(gp4::Regs* r) {
    gp4::Regs c = *r;

    gp4::call_regs(0x0040baa8, c);

    uint32_t half = (static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0062250c) * GP4_GLOBAL(int32_t, 0x00622778)) >> 17);
    half = (half + 1u) >> 1;
    if (half == 0u) {
        half = 1u;
    }
    GP4_GLOBAL(uint32_t, 0x006227ac) = half;
    uint32_t size = half << 1;
    GP4_GLOBAL(uint32_t, 0x006227b0) = size;

    const uint16_t angle = GP4_GLOBAL(uint16_t, 0x00622514);

    uint16_t mirrored = static_cast<uint16_t>(0x4000u - angle);
    if (static_cast<int16_t>(mirrored) < 0) {
        mirrored = static_cast<uint16_t>(0u - mirrored);
    }
    const int32_t entryA = GP4_ARRAY(int16_t, 0x007c5c5c)[((mirrored >> 2) & 0xfffeu) >> 1];

    uint16_t direct = angle;
    if (static_cast<int16_t>(direct) < 0) {
        direct = static_cast<uint16_t>(0u - direct);
    }
    const int32_t entryB = GP4_ARRAY(int16_t, 0x007c5c5c)[((direct >> 2) & 0xfffeu) >> 1];

    const uint32_t stepA = static_cast<uint32_t>(-(entryA >> 3));
    const uint32_t stepB = static_cast<uint32_t>(-(entryB >> 3));

    GP4_GLOBAL(uint32_t, 0x006227b4) = 0;
    GP4_GLOBAL(uint32_t, 0x006227b8) = 0;
    GP4_GLOBAL(uint32_t, 0x006227bc) = 0;
    GP4_GLOBAL(uint32_t, 0x006227c0) = 0;

    c.esi = stepA;
    c.edi = stepB;
    c.ebp = size;
    c.eax = stepB;
    c.ecx = static_cast<uint32_t>(entryB);
    gp4::call_regs(0x0040b99a, c);
    gp4::call_regs(0x0040b955, c);

    const int32_t divisor = static_cast<int32_t>(size << 9);
    for (uint32_t i = 0; i != 2u; ++i) {
        int32_t rem;
        int32_t q = gp4::idiv64(static_cast<int64_t>(GP4_ARRAY(uint32_t, 0x006227b4)[i]), divisor, &rem);
        if (q < 0) {
            q = 0;
        }
        if (q > 0xb0) {
            q = 0xb0;
        }
        GP4_ARRAY(int32_t, 0x0063b8a0)[i] = q;

        q = gp4::idiv64(static_cast<int64_t>(GP4_ARRAY(uint32_t, 0x006227bc)[i]), divisor, &rem);
        if (q < 0) {
            q = 0;
        }
        if (q > 0x70) {
            q = 0x70;
        }
        GP4_ARRAY(int32_t, 0x0063b8a8)[i] = q;
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0040bb26, FUN_0040bb26, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040b99a+map_0040c43b+state_0040bb26")
