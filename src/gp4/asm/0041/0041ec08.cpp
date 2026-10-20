#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul by a Q14 factor: low dword of (a * b) >> 14 (imul r/m32 followed by shrd eax, edx, 14).
static inline uint32_t mulQ14(uint32_t a, int32_t b) {
    const int64_t product = (int64_t)(int32_t)a * (int64_t)b;
    return (uint32_t)((uint64_t)product >> 14);
}

// One Hermite/B-spline style cubic step over the four samples 0x637ff0..0x637ffc (filled by 0x41eacf and
// advanced by 0x41eba5) for the segment record in edi; eax = Q14 parameter, edx = lateral offset.
// FUNCTION: GP4 0x0041ec08
void __cdecl FUN_0041ec08(gp4::Regs* r) {
    const int32_t param = (int32_t)r->eax;
    int32_t offset = (int32_t)r->edx;
    const uint32_t record = r->edi;
    void* rec = reinterpret_cast<void*>(static_cast<uintptr_t>(record));

    GP4_GLOBAL(int32_t, 0x00637fec) = param;
    GP4_GLOBAL(int32_t, 0x00627854) = 0;

    // Clamp the offset to the lane edge words (+0x60 left, +0x62 right); remember the excess (<= 0x15000).
    int32_t limit;
    bool outside;
    if (offset >= 0) {
        limit = (int32_t)((GP4_FIELD(uint16_t, rec, 0x62) - 0x100u) << 8);
        outside = offset > limit;
    } else {
        limit = (int32_t)((0u - (GP4_FIELD(uint16_t, rec, 0x60) - 0x100u)) << 8);
        outside = offset < limit;
    }
    if (outside) {
        uint32_t excess = (uint32_t)offset - (uint32_t)limit;
        if ((int32_t)excess >= 0) {
            if (excess >= 0x15000u) excess = 0x15000u;
        } else {
            excess = 0u - excess;
            if (excess >= 0x15000u) excess = 0x15000u;
            excess = 0u - excess;
        }
        GP4_GLOBAL(uint32_t, 0x00627854) = excess;
        offset = limit;
    }

    gp4::Regs c = *r;
    c.edx = (uint32_t)offset;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0041eacf, c);
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0041eba5, c);

    const uint32_t p0 = GP4_GLOBAL(uint32_t, 0x00637ff0);
    const uint32_t p1 = GP4_GLOBAL(uint32_t, 0x00637ff4);
    const uint32_t p2 = GP4_GLOBAL(uint32_t, 0x00637ff8);
    const uint32_t p3 = GP4_GLOBAL(uint32_t, 0x00637ffc);
    const uint32_t p1x3 = p1 * 3u;
    const uint32_t p2x3 = p2 * 3u;

    const uint32_t c0 = p3 - p0 + p1x3 - p2x3;
    GP4_GLOBAL(uint32_t, 0x00638000) = c0;
    const uint32_t c1 = p0 * 3u - p1x3 - p1x3 + p2x3;
    GP4_GLOBAL(uint32_t, 0x00638004) = c1;
    const uint32_t c2 = ((p2x3 - p0) << 1) - p1x3 - p3;
    GP4_GLOBAL(uint32_t, 0x00638008) = c2;
    const uint32_t c3 = p1x3 << 1;
    GP4_GLOBAL(uint32_t, 0x0063800c) = c3;

    uint32_t acc = mulQ14(GP4_GLOBAL(uint32_t, 0x00638000), GP4_GLOBAL(int32_t, 0x00637fec));
    acc += GP4_GLOBAL(uint32_t, 0x00638004);
    acc = mulQ14(acc, GP4_GLOBAL(int32_t, 0x00637fec));
    acc += GP4_GLOBAL(uint32_t, 0x00638008);
    acc = mulQ14(acc, GP4_GLOBAL(int32_t, 0x00637fec));
    acc += GP4_GLOBAL(uint32_t, 0x0063800c);
    const int32_t quotient = (int32_t)acc / 6;

    uint32_t result = (uint32_t)quotient;
    uint32_t flags = 0x44u;  // test [0x627854], -1 with a zero value: PF | ZF
    if (GP4_GLOBAL(int32_t, 0x00627854) != 0) {
        const uint32_t base = GP4_FIELD(uint32_t, rec, 0x150);
        const uint32_t top = GP4_FIELD(uint32_t, rec, 0x2b0);
        const uint32_t blended = mulQ14(top - base, GP4_GLOBAL(int32_t, 0x00637fec)) + base;
        GP4_GLOBAL(uint32_t, 0x006f2bf4) = blended;
        gp4::x87::Ext v = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
        v = v * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2a04));
        v = v * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00627854));
        const int32_t scaled = gp4::x87::fistp32(v);
        GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;
        result = (uint32_t)quotient - (uint32_t)scaled;
        flags = subFlags32((uint32_t)quotient, (uint32_t)scaled);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041ec08, FUN_0041ec08, "regs(eax:i32, edx:i32, edi:rec[0x8943a0,0x160,0..25]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+multiply_0040ce7b+words_008943a0+dsmall_0041ec08")
