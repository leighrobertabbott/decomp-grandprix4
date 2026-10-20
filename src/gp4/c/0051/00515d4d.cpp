#include <gp4/gp4.h>
#include <gp4/flags.h>

// Takes the low four bits of the value 0x0040f8d3 returns, clamps them to at most 9 and stores the result
// at 0x00866128. eax ends with the clamped value; the flags are those of `cmp eax, 9` (the following
// `mov eax, 9` and the store leave them alone).
// FUNCTION: GP4 0x00515d4d
void __cdecl FUN_00515d4d(gp4::Regs* r) {
    uint32_t value = gp4::cdecl_<0x0040f8d3, uint32_t>() & 0xfu;
    const uint32_t flags = gp4::flags::sub32(value, 9u);
    if (static_cast<int32_t>(value) > 9) {
        value = 9u;
    }
    GP4_GLOBAL(uint32_t, 0x00866128) = value;
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00515d4d, FUN_00515d4d, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=val_0062fbf8")
