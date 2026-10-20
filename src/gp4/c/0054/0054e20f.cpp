#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads a pair of state values after the empty hook 0x0054d6f2: with n = the incoming eax, ebp = n -
// (n >> 1) + [0x00833cc5] and ebx = [0x00833cc5] - (n >> 1); eax = [0x00833cc9] and ecx = eax +
// [0x00833f58]. The flags are those of that final add. Reads eax at entry, so the register
// convention is needed.
// FUNCTION: GP4 0x0054e20f
void __cdecl FUN_0054e20f(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x0054d6f2, c);

    const uint32_t n = c.eax;
    const uint32_t half = n >> 1;
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x00833cc5);
    const uint32_t eax = GP4_GLOBAL(uint32_t, 0x00833cc9);
    const uint32_t addend = GP4_GLOBAL(uint32_t, 0x00833f58);

    r->ebp = (n - half) + base;
    r->ebx = base - half;
    r->eax = eax;
    r->ecx = eax + addend;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(eax, addend));
}
GP4_IMPL(0x0054e20f, FUN_0054e20f, "regs(eax:u32) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
