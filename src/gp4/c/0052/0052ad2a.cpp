#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: eax = the time value [0x00a69878] (milliseconds), formatted into the buffer at
// 0x00823100 by 0x0052ad37 (ebp = -1, edi = the buffer, then 0x0052ad57). popal restores every
// register; the flags are those 0x0052ad37 leaves.
// FUNCTION: GP4 0x0052ad2a
void __cdecl FUN_0052ad2a(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x0052ad37, call);
    r->eflags = gp4::flags::merge(r->eflags, call.eflags);
}
GP4_IMPL(0x0052ad2a, FUN_0052ad2a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0052ad37")
