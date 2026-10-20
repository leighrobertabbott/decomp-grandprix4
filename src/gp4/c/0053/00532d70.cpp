#include <gp4/gp4.h>
#include <gp4/flags.h>

// `cmp byte [0x007ad869], 0`: when the byte is zero the countdown step 0x00541f14 runs (it decrements
// [0x007a9780] and updates the mode bytes 0x007a9790..2); otherwise nothing happens. Registers and flags
// are those of the compare or of the callee.
// FUNCTION: GP4 0x00532d70
void __cdecl FUN_00532d70(gp4::Regs* r) {
    const uint32_t flag = GP4_GLOBAL(uint8_t, 0x007ad869);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(flag, 0u));
    if (flag == 0u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00541f14, c);
        *r = c;
    }
}
GP4_IMPL(0x00532d70, FUN_00532d70, "regs(eax:u32, ebx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00541f51")
