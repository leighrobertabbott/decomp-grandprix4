#include <gp4/gp4.h>
#include <gp4/flags.h>

// 0x00833cc5 (unaligned dword) += the dword at 0x00833f64; eax keeps the addend and the
// flags are those of the `add dword [0x833cc5], eax`.
// FUNCTION: GP4 0x0054d298
void __cdecl FUN_0054d298(gp4::Regs* r) {
    const uint32_t addend = GP4_GLOBAL(uint32_t, 0x00833f64);
    const uint32_t old = GP4_GLOBAL(uint32_t, 0x00833cc5);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = old + addend;
    r->eax = addend;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(old, addend));
}
GP4_IMPL(0x0054d298, FUN_0054d298, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
