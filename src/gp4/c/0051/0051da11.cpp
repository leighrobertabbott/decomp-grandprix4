#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the dword at 0x007a93c8 is nonzero, runs the stdcall routine 0x005064f0 on it (the value is the
// only argument). The flags are those of `test dword [0x7a93c8], -1`, or the callee's when it ran;
// eax, ecx and edx come back as the callee leaves them.
// FUNCTION: GP4 0x0051da11
void __cdecl FUN_0051da11(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007a93c8);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
    if (value != 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        const uint32_t args[1] = {value};
        gp4::call_regs_stdcall(0x005064f0, c, args);
        *r = c;
    }
}
GP4_IMPL(0x0051da11, FUN_0051da11, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
