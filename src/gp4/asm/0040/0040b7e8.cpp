#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Slew-rate limiter for a pair of values (DAT_00622784 = target-following value, DAT_00622788 = level).
// eax = the new requested value, edx (ebx) = the step class. DAT_00622784 moves towards eax by at most a
// table step: up by DAT_00622688[class], down by DAT_006226a0[min(cur >> 12, 11)] when eax is 0 and by
// DAT_006226e0[...] otherwise. Then, for class >= 3, DAT_00622788 decays by DAT_00622760[class] (floor 0);
// below 3 it grows by DAT_00622720[min(cur >> 12, 11)] capped at cur + 0x2000 and 0x7000. Returns the new
// level in eax with the flags of the last compare (the one against 0 or against 0x7000); ebx/ecx/edx are
// preserved.
// FUNCTION: GP4 0x0040b7e8
void __cdecl FUN_0040b7e8(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(r->eax);
    const int32_t cls = static_cast<int32_t>(r->edx);

    const int32_t current = GP4_GLOBAL(int32_t, 0x00622784);
    if (value != current) {
        if (value >= current) {
            const int32_t limit = static_cast<int32_t>(static_cast<uint32_t>(current) + static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x00622688)[cls]));
            value = (limit <= value) ? limit : value;
        } else if (value != 0) {
            int32_t slot = static_cast<int32_t>(static_cast<uint32_t>(current) >> 12);
            if (slot > 0xb) {
                slot = 0xb;
            }
            const int32_t candidate = static_cast<int32_t>(static_cast<uint32_t>(current) - static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x006226e0)[slot]));
            value = (candidate >= value) ? candidate : value;
        } else {
            int32_t slot = static_cast<int32_t>(static_cast<uint32_t>(current) >> 12);
            if (slot > 0xb) {
                slot = 0xb;
            }
            const int32_t candidate = static_cast<int32_t>(static_cast<uint32_t>(current) - static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x006226a0)[slot]));
            value = (candidate >= 0) ? candidate : 0;
        }
    }
    GP4_GLOBAL(int32_t, 0x00622784) = value;

    uint32_t flags;
    int32_t level;
    if (cls >= 3) {
        level = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00622788)) - static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x00622760)[cls]));
        flags = subFlags32(static_cast<uint32_t>(level), 0u);
        if (level < 0) {
            level = 0;
        }
    } else {
        int32_t slot = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00622784)) >> 12);
        if (slot > 0xb) {
            slot = 0xb;
        }
        level = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00622788)) + static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x00622720)[slot]));
        const int32_t ceiling = static_cast<int32_t>(static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00622784)) + 0x2000u);
        if (level > ceiling) {
            level = ceiling;
        }
        flags = subFlags32(static_cast<uint32_t>(level), 0x7000u);
        if (level > 0x7000) {
            level = 0x7000;
        }
    }
    GP4_GLOBAL(int32_t, 0x00622788) = level;

    r->eax = static_cast<uint32_t>(level);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040b7e8, FUN_0040b7e8, "regs(eax:i32, edx:i32[0..8]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
