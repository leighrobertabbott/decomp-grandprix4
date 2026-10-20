#include <gp4/gp4.h>
#include <gp4/flags.h>

// push eax; push edi; call 0x0054f629; pop edi; pop eax; ret
// Runs the 0x0054f629 text-block builder (a pushal/popal routine that reads no register); eax and edi
// are restored by the pops, so only the flags of the callee's last flag writer are left.
// FUNCTION: GP4 0x0054f54b
void __cdecl FUN_0054f54b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054f629, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054f54b, FUN_0054f54b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
