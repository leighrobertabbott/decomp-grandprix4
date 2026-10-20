#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores max(0, [0x679b04] - [0x66d814]) (signed) at 0x00866080 and leaves it in eax. The flags are
// those of `cmp eax, 0` on the difference (the `mov eax, 0` of the clamp does not touch them).
// The routine then falls through into the empty routine at 0x00515b63 (a single `ret`).
// FUNCTION: GP4 0x00515b49
void __cdecl FUN_00515b49(gp4::Regs* r) {
    int32_t value = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00679b04) - GP4_GLOBAL(uint32_t, 0x0066d814));
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(value), 0u);
    if (value < 0) {
        value = 0;
    }
    GP4_GLOBAL(int32_t, 0x00866080) = value;
    r->eax = static_cast<uint32_t>(value);
    r->eflags = gp4::flags::merge(r->eflags, flags);
    gp4::call_regs(0x00515b63, *r);
}
GP4_IMPL(0x00515b49, FUN_00515b49, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
