#include <gp4/gp4.h>
#include <gp4/flags.h>

// `mov ebp, 0; mov edi, 0x823100; call 0x0052ad57; ret`: formats the time value in eax (milliseconds,
// hours:minutes:seconds.milliseconds with the script fragments 0x3d/0x3e/0x3f) into the buffer at
// 0x00823100 with ebp = 0 (no leading zero fields). 0x0052ad57 saves and restores every register but
// edi, which ends at the terminating NUL; the flags are those it leaves.
// FUNCTION: GP4 0x0052ad47
void __cdecl FUN_0052ad47(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebp = 0u;
    c.edi = 0x00823100u;
    gp4::call_regs(0x0052ad57, c);
    r->ebp = c.ebp;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052ad47, FUN_0052ad47, "regs(eax:u32) -> (ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0052ad37")
