#include <gp4/gp4.h>
#include <gp4/flags.h>

// cmp dword [0x66d814], 0x1b630 ; jne next ; ret: both branch directions reach the same ret,
// so the only effect is the flags of the compare.
// FUNCTION: GP4 0x005244cd
void __cdecl FUN_005244cd(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x0066d814);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(value, 0x0001b630u));
}
GP4_IMPL(0x005244cd, FUN_005244cd, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
