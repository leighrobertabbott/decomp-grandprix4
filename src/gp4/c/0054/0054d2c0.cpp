#include <gp4/gp4.h>
#include <gp4/flags.h>

// The (unaligned) dword at 0x00833cc9 loses the sum of the dwords 0x00833f58 and 0x00833f70; eax
// keeps the sum. The flags are those of the final `sub [0x833cc9], eax`.
// FUNCTION: GP4 0x0054d2c0
void __cdecl FUN_0054d2c0(gp4::Regs* r) {
    const uint32_t sum = GP4_GLOBAL(uint32_t, 0x00833f58) + GP4_GLOBAL(uint32_t, 0x00833f70);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00833cc9);
    GP4_GLOBAL(uint32_t, 0x00833cc9) = before - sum;
    r->eax = sum;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(before, sum));
}
GP4_IMPL(0x0054d2c0, FUN_0054d2c0, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
