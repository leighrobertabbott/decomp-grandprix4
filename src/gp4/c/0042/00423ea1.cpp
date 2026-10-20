#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `sar value, 1` in the pinned oracle model: CF = bit 0, OF = 0, AF = 0.
static inline uint32_t sarFlags1(uint32_t value, uint32_t result) {
    return (value & 1u) | parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);
}

// current + ((sum * DAT_00607fa0) >> 8), then clamped to +-DAT_0060972c (unsigned magnitude compare).
static inline uint32_t advanceClamped(uint32_t current, int32_t sum) {
    const int64_t product = static_cast<int64_t>(sum) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00607fa0));
    uint32_t value = static_cast<uint32_t>(product >> 8) + current;
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= GP4_GLOBAL(uint32_t, 0x0060972c)) {
            magnitude = GP4_GLOBAL(uint32_t, 0x0060972c);
        }
        return 0u - magnitude;
    }
    if (value >= GP4_GLOBAL(uint32_t, 0x0060972c)) {
        value = GP4_GLOBAL(uint32_t, 0x0060972c);
    }
    return value;
}

// FUNCTION: GP4 0x00423ea1
// Integrates the vector (DAT_00607f40, DAT_00607f44, DAT_00607f48) by the sums
// (DAT_006081e0 + DAT_006081d4, DAT_006081e4 + DAT_006081d8, DAT_006081e8 + DAT_006081dc) scaled by
// DAT_00607fa0 >> 8, clamps each component to +-DAT_0060972c, limits the vector length to DAT_006080d4
// with 0x00402cd0 and stores the mean of the old and new vector to DAT_006081ec/f0/f4.
// Leaves eax = the last mean, ebp = new y, edx = new z (ecx is preserved); the flags are those of the
// final `sar eax, 1`.
void __cdecl FUN_00423ea1(gp4::Regs* r) {
    const uint32_t oldX = GP4_GLOBAL(uint32_t, 0x00607f40);
    const uint32_t oldY = GP4_GLOBAL(uint32_t, 0x00607f44);
    const uint32_t oldZ = GP4_GLOBAL(uint32_t, 0x00607f48);

    GP4_GLOBAL(uint32_t, 0x00607f40) = advanceClamped(GP4_GLOBAL(uint32_t, 0x00607f40),
        static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006081e0) + GP4_GLOBAL(uint32_t, 0x006081d4)));
    GP4_GLOBAL(uint32_t, 0x00607f44) = advanceClamped(GP4_GLOBAL(uint32_t, 0x00607f44),
        static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006081e4) + GP4_GLOBAL(uint32_t, 0x006081d8)));
    GP4_GLOBAL(uint32_t, 0x00607f48) = advanceClamped(GP4_GLOBAL(uint32_t, 0x00607f48),
        static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006081e8) + GP4_GLOBAL(uint32_t, 0x006081dc)));

    gp4::Regs limit = *r;
    limit.st_in = 0;
    limit.st_out = 0;
    limit.eax = GP4_GLOBAL(uint32_t, 0x00607f40);
    limit.ebp = GP4_GLOBAL(uint32_t, 0x00607f44);
    limit.edx = GP4_GLOBAL(uint32_t, 0x00607f48);
    limit.ecx = GP4_GLOBAL(uint32_t, 0x006080d4);
    gp4::call_regs(0x00402cd0, limit);
    GP4_GLOBAL(uint32_t, 0x00607f40) = limit.eax;
    GP4_GLOBAL(uint32_t, 0x00607f44) = limit.ebp;
    GP4_GLOBAL(uint32_t, 0x00607f48) = limit.edx;

    uint32_t sum = oldZ + GP4_GLOBAL(uint32_t, 0x00607f48);
    GP4_GLOBAL(int32_t, 0x006081f4) = static_cast<int32_t>(sum) >> 1;
    sum = oldY + GP4_GLOBAL(uint32_t, 0x00607f44);
    GP4_GLOBAL(int32_t, 0x006081f0) = static_cast<int32_t>(sum) >> 1;
    sum = oldX + GP4_GLOBAL(uint32_t, 0x00607f40);
    const int32_t mean = static_cast<int32_t>(sum) >> 1;
    GP4_GLOBAL(int32_t, 0x006081ec) = mean;

    r->eax = static_cast<uint32_t>(mean);
    r->ebp = limit.ebp;
    r->edx = limit.edx;
    r->eflags = (r->eflags & ~0x8d5u) | sarFlags1(sum, static_cast<uint32_t>(mean));
}
GP4_IMPL(0x00423ea1, FUN_00423ea1, "regs() -> (eax, ebp, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
