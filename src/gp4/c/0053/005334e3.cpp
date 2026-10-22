#include <gp4/gp4.h>
#include <gp4/flags.h>

// Front-end screen entry: 0x00527f80 (an empty `ret`) is called with eax = 0x315, then esi = the static item
// chain 0x0084949a is flagged with the mask eax = 4 when the byte [0x007aafd8] is 0xff, else 0 (0x0053e4db),
// and the screen loop 0x0053c8af runs. One register block is threaded through the calls; the flags before the
// walk are those of `cmp byte [0x7aafd8], 0xff`, or of `xor eax, eax` when the byte differs.
// FUNCTION: GP4 0x005334e3
void __cdecl FUN_005334e3(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::cdecl_<0x00527f80, void>();
    c.eax = 0x315u;
    c.esi = 0x0084949au;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007aafd8);
    if (mode == 0xffu) {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(mode, 0xffu));
        c.eax = 4u;
    } else {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(0u));
        c.eax = 0u;
    }
    gp4::call_regs(0x0053e4db, c);
    gp4::call_regs(0x0053c8af, c);
    *r = c;
}
GP4_IMPL(0x005334e3, FUN_005334e3, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0044535c; emu=skip:win32 PeekMessageA")
