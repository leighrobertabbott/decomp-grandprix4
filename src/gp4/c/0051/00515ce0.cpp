#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores min([0x007ad8a0] & 0xf, 2) in the dword 0x008660bc and loads the constants ebx = 4, ebp = 7,
// edx = 0. eax ends as the stored value; the flags are those of `cmp eax, 2` on the masked value
// (the clamping `mov` touches none).
// FUNCTION: GP4 0x00515ce0
void __cdecl FUN_00515ce0(gp4::Regs* r) {
    uint32_t value = GP4_GLOBAL(uint16_t, 0x007ad8a0) & 0xfu;
    const uint32_t arith = gp4::flags::sub32(value, 2u);
    if (static_cast<int32_t>(value) > 2) {
        value = 2u;
    }
    GP4_GLOBAL(uint32_t, 0x008660bc) = value;
    r->eax = value;
    r->ebx = 4u;
    r->ebp = 7u;
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00515ce0, FUN_00515ce0, "regs() -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
