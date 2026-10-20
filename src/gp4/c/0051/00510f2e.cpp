#include <gp4/gp4.h>

// Derives a clamped divisor-based value: 0x100 * [0x88f63c] / (max(12, [0x7ad8b8] >> 8) << 8),
// clamped to 2..0x4b, stored in 0x007a9784 (and returned in eax).
// FUNCTION: GP4 0x00510f2e
int __cdecl FUN_00510f2e() {
    uint32_t divisor = GP4_GLOBAL(uint32_t, 0x007ad8b8) >> 8;
    if ((int32_t)divisor < 0xc) {
        divisor = 0xc;
    }
    uint32_t rem;
    int32_t value = (int32_t)gp4::div64((uint64_t)GP4_GLOBAL(uint32_t, 0x0088f63c) * 0x100u, divisor << 8, &rem);
    if (value < 2) {
        value = 2;
    }
    if (value > 0x4b) {
        value = 0x4b;
    }
    GP4_GLOBAL(int32_t, 0x007a9784) = value;
    return value;
}
GP4_IMPL(0x00510f2e, FUN_00510f2e, "cdecl() -> i32; globals=fuzz:i32")
