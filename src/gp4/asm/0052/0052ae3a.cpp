#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: unless eax is the "no value" marker 0xffff, formats the date/time in eax through
// 0x0052ae53 (text into the buffer 0x00823100) and then queues that buffer with 0x0054d211
// (esi = 0x00823100). popal restores every register, so only the flags remain: those of
// `cmp eax, 0xffff` when skipped, else those of 0x0054d211 (the `inc` of its counter).
// FUNCTION: GP4 0x0052ae3a
void __cdecl FUN_0052ae3a(gp4::Regs* r) {
    if (r->eax == 0xffffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->eax, 0xffffu));
        return;
    }
    gp4::Regs c = *r;
    gp4::call_regs(0x0052ae53, c);
    c.esi = 0x00823100u;
    gp4::call_regs(0x0054d211, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052ae3a, FUN_0052ae3a, "regs(eax:u32{=0xffff,0,1,100,5000,12345,0x10000,0x1ffff,20000}, edx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846")
