#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the path "C:\" + "\GPSAVES" (both through the string expander 0x0054d7c0) in the buffer
// 0x00823100 and makes it the current directory (stdcall thunk 0x005a48a6, argument: the path). When that
// call does not return 1, the directory is created with 0x005a48ac (path, 0). pushal/popal around the body
// restore every register; the flags are those of the `cmp eax, 1`, or of the second call when it ran.
// FUNCTION: GP4 0x00537132
void __cdecl FUN_00537132(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.esi = 0x0081e6eau;
    c.edi = 0x00823100u;
    gp4::call_regs(0x0054d7c0, c);
    c.esi = 0x0081e9ffu;
    gp4::call_regs(0x0054d7c0, c);

    const uint32_t chdirArgs[1] = {0x00823100u};
    gp4::call_regs_stdcall(0x005a48a6, c, chdirArgs);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 1u));
    if (c.eax != 1u) {
        const uint32_t mkdirArgs[2] = {0x00823100u, 0u};
        gp4::call_regs_stdcall(0x005a48ac, c, mkdirArgs);
    }
    r->eflags = c.eflags;
}
GP4_IMPL(0x00537132, FUN_00537132, "regs() -> (cf, pf, af, zf, sf, of)")
