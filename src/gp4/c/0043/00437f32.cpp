#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t res) {
    return (res < a ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ res) & (b ^ res)) >> 31) << 11);
}

// FUNCTION: GP4 0x00437f32
// Tests the point (DAT_00629de4, DAT_00629de8) against two edge records: for each of
// the offsets (DAT_006f3d6c/70, then DAT_006f3d74/78) it stores the difference vector
// in DAT_00934df0/DAT_00934df4 and takes the fixed-point (>> 14) dot product with the
// normal (DAT_00629df0, DAT_00629dec). Returns CF set (stc) when the first dot product
// is negative, or when the second is non-negative; CF clear otherwise. The flags are
// those of the last dot-product add. eax and ebx are preserved; edx is left holding the
// high half of the last imul.
void __cdecl FUN_00437f32(gp4::Regs* r) {
    const uint32_t px = GP4_GLOBAL(uint32_t, 0x00629de4);
    const uint32_t py = GP4_GLOBAL(uint32_t, 0x00629de8);
    uint32_t fl = r->eflags & ~0x8D5u;

    GP4_GLOBAL(uint32_t, 0x00934df0) = px - GP4_GLOBAL(uint32_t, 0x006f3d6c);
    GP4_GLOBAL(uint32_t, 0x00934df4) = py - GP4_GLOBAL(uint32_t, 0x006f3d70);
    int64_t p1 = (int64_t)GP4_GLOBAL(int32_t, 0x00629dec) * (int64_t)GP4_GLOBAL(int32_t, 0x00934df4);
    const uint32_t b1 = (uint32_t)((uint64_t)p1 >> 14);
    int64_t p2 = (int64_t)GP4_GLOBAL(int32_t, 0x00629df0) * (int64_t)GP4_GLOBAL(int32_t, 0x00934df0);
    uint32_t a1 = (uint32_t)((uint64_t)p2 >> 14);
    uint32_t sum = a1 + b1;
    uint32_t edx = (uint32_t)((uint64_t)p2 >> 32);
    if ((int32_t)sum < 0) {
        r->edx = edx;
        r->eflags = fl | addFlags(a1, b1, sum) | 1u;
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00934df0) = px - GP4_GLOBAL(uint32_t, 0x006f3d74);
    GP4_GLOBAL(uint32_t, 0x00934df4) = py - GP4_GLOBAL(uint32_t, 0x006f3d78);
    p1 = (int64_t)GP4_GLOBAL(int32_t, 0x00629dec) * (int64_t)GP4_GLOBAL(int32_t, 0x00934df4);
    const uint32_t b2 = (uint32_t)((uint64_t)p1 >> 14);
    p2 = (int64_t)GP4_GLOBAL(int32_t, 0x00629df0) * (int64_t)GP4_GLOBAL(int32_t, 0x00934df0);
    const uint32_t a2 = (uint32_t)((uint64_t)p2 >> 14);
    sum = a2 + b2;
    edx = (uint32_t)((uint64_t)p2 >> 32);
    fl |= addFlags(a2, b2, sum);
    fl = (fl & ~1u) | ((sum >> 31) ^ 1u);
    r->edx = edx;
    r->eflags = fl;
}
GP4_IMPL(0x00437f32, FUN_00437f32, "regs() -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
