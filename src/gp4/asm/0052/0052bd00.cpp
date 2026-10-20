#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t entry_address(uint32_t index) {
    return index + 0x0081dc63u;
}

// Looks up byte al in the table at 0x0081dc63 and reports whether it differs from 0xff: `cmp byte
// [eax], 0xff` then `stc` (entry used, != 0xff) or `clc` (== 0xff). eax is pushed/popped and so
// unchanged; the flags are those of the byte compare with CF replaced by the result.
// FUNCTION: GP4 0x0052bd00
void __cdecl FUN_0052bd00(gp4::Regs* r) {
    const uint8_t value = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(entry_address(r->eax & 0xffu)));
    const uint32_t carry = (value != 0xffu) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::sub8(value, 0xffu) & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0052bd00, FUN_0052bd00, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
