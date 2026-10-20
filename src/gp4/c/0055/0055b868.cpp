#include <gp4/gp4.h>
#include <gp4/x87.h>

// `test ah, 0x41 ; jp`: the C0/C3 bits of an x87 compare status have even parity (both clear or both set).
static inline bool parityEven(uint16_t status) {
    const uint32_t bits = status & 0x4100u;
    return bits == 0u || bits == 0x4100u;
}

// Returns 1 when the float difference a - b (kept at extended precision on the x87 stack) lies within the
// closed range [0x005f0fec, 0x005f0fe8]; 0 otherwise (an unordered compare gives 0).
// FUNCTION: GP4 0x0055b868
int __cdecl FUN_0055b868(float a, float b) {
    using namespace gp4::x87;
    const Ext difference = ext_f32(a) - ext_f32(b);
    if (parityEven(fcom(ext_f32(GP4_GLOBAL(float, 0x005f0fecu)), difference))) {
        return 0;
    }
    if (parityEven(fcom(difference, ext_f32(GP4_GLOBAL(float, 0x005f0fe8u))))) {
        return 0;
    }
    return 1;
}
GP4_IMPL(0x0055b868, FUN_0055b868, "cdecl(f32, f32) -> i32; globals=fuzz:i32")
