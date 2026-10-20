#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t entry_address(uint32_t index) {
    return index * 0x1cu + 0x00b1c8e8u;
}

// Tests whether the first dword of entry eax (0x1c bytes each) of the table at 0x00b1c8e8 is
// non-zero: `cmp dword [eax], 0` then `stc` (non-zero) or `clc` (zero). eax is pushed/popped and so
// unchanged; the flags are those of the compare with CF replaced by the result.
// FUNCTION: GP4 0x0051f27f
void __cdecl FUN_0051f27f(gp4::Regs* r) {
    const uint32_t value = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry_address(r->eax)));
    const uint32_t carry = (value != 0u) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::sub32(value, 0u) & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0051f27f, FUN_0051f27f, "regs(eax:u32[0..25]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
