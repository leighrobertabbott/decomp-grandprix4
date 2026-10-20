#include <gp4/gp4.h>
#include <gp4/flags.h>

// 0x00833cc5 (unaligned dword) -= the dword at 0x00833f64; eax keeps the subtrahend and the
// flags are those of the `sub dword [0x833cc5], eax`.
// FUNCTION: GP4 0x0054d28c
void __cdecl FUN_0054d28c(gp4::Regs* r) {
    const uint32_t sub = GP4_GLOBAL(uint32_t, 0x00833f64);
    const uint32_t old = GP4_GLOBAL(uint32_t, 0x00833cc5);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = old - sub;
    r->eax = sub;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(old, sub));
}
GP4_IMPL(0x0054d28c, FUN_0054d28c, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
