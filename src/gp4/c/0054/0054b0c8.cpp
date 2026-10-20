#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the bytes at 0x007aafe0 and 0x007aafe1 (`cmp al, [0x7aafe1]`; eax is pushed/popped around
// it) and ends with `stc` when they differ, `clc` when equal. The flags are those of the compare with
// CF replaced by the result.
// FUNCTION: GP4 0x0054b0c8
void __cdecl FUN_0054b0c8(gp4::Regs* r) {
    const uint8_t a = GP4_GLOBAL(uint8_t, 0x007aafe0);
    const uint8_t b = GP4_GLOBAL(uint8_t, 0x007aafe1);
    const uint32_t carry = (a != b) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::sub8(a, b) & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0054b0c8, FUN_0054b0c8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
