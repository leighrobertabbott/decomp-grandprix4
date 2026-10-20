#include <gp4/gp4.h>

// idiv with edx = 0 divides the zero-extended 32-bit value (a << 14) by the signed divisor.
// It is written with 32-bit unsigned division so no 64-bit helper call is emitted; the
// results are identical whenever the original idiv does not fault.
// FUNCTION: GP4 0x0041931d
int32_t __cdecl FUN_0041931d() {
    const uint32_t scaled = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0060150c)) << 14;
    const int32_t divisor = GP4_GLOBAL(int32_t, 0x0060351c);
    uint32_t magnitude;
    if (divisor >= 0) {
        magnitude = scaled / static_cast<uint32_t>(divisor);
    } else {
        magnitude = 0u - scaled / (0u - static_cast<uint32_t>(divisor));
    }
    const int32_t quotient = static_cast<int32_t>(magnitude);
    GP4_GLOBAL(int32_t, 0x00603518) = quotient;
    return quotient;
}
GP4_IMPL(0x0041931d, FUN_0041931d, "cdecl() -> i32; globals=layout=state_0041931d")
