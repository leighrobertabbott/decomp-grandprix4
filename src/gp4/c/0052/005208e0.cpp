#include <gp4/gp4.h>
#include <gp4/flags.h>

// Subtracts 8 from the signed counter at 0x0066d834 and clamps it at zero. eax ends with the stored
// value; the flags are those of `cmp eax, 0` on the unclamped difference (the clamping `mov` leaves them).
// FUNCTION: GP4 0x005208e0
void __cdecl FUN_005208e0(gp4::Regs* r) {
    uint32_t value = GP4_GLOBAL(uint32_t, 0x0066d834) - 8u;
    const uint32_t flags = gp4::flags::sub32(value, 0u);
    if (static_cast<int32_t>(value) < 0) {
        value = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x0066d834) = value;
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005208e0, FUN_005208e0, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
