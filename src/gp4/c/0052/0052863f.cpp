#include <gp4/gp4.h>
#include <gp4/flags.h>

// Link-session poll: only when the byte [0x007aafd3] is 0xff, runs 0x0054a848, then 0x0054ac8b with al = 0xf (the
// upper bits of eax are kept), then the receive pass 0x0054a2c3 (which reaches the message pump). One register
// block is threaded through the three calls. Otherwise only the flags of `cmp byte [0x7aafd3], 0xff` change.
// FUNCTION: GP4 0x0052863f
void __cdecl FUN_0052863f(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007aafd3);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0xffu));
    if (mode != 0xffu) {
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054a848, c);
    c.eax = (c.eax & 0xffffff00u) | 0xfu;
    gp4::call_regs(0x0054ac8b, c);
    gp4::call_regs(0x0054a2c3, c);
    *r = c;
}
GP4_IMPL(0x0052863f, FUN_0052863f, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=mode_007aafd3; emu=skip:win32 PeekMessageA")
