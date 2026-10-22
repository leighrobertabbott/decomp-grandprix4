#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-frame effects tail: when the byte [0x007ad869] is nonzero the stub 0x004a60c0 (`xor eax, eax`) runs
// first, then 0x00510bf3 (pushal wrapper) and 0x005501ae (pushal wrapper). One register block is threaded
// through the calls; the flags are those of `cmp byte [0x7ad869], 0` until a callee overwrites them.
// FUNCTION: GP4 0x0055176b
void __cdecl FUN_0055176b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007ad869);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(enabled, 0u));
    if (enabled != 0u) {
        gp4::call_regs(0x004a60c0, c);
    }
    gp4::call_regs(0x00510bf3, c);
    gp4::call_regs(0x005501ae, c);
    *r = c;
}
GP4_IMPL(0x0055176b, FUN_0055176b, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=box_00550798")
