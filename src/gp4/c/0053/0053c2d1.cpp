#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: expands the text scripts 0x6ea and 5 into the static buffer 0x00823100 with the
// interpreter 0x0054d798 (esi = script id, edi = output), then popal restores every register and the
// item record at esi is drawn with 0x0053c135. The flags passed on are those the interpreter leaves;
// afterwards they are 0x0053c135's.
// FUNCTION: GP4 0x0053c2d1
void __cdecl FUN_0053c2d1(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x00823100u;
    c.esi = 0x6eau;
    gp4::call_regs(0x0054d798, c);
    c.esi = 5u;
    gp4::call_regs(0x0054d798, c);

    gp4::Regs d = *r;
    d.st_in = 0;
    d.st_out = 0;
    d.eflags = gp4::flags::merge(r->eflags, c.eflags);
    gp4::call_regs(0x0053c135, d);
    r->eflags = gp4::flags::merge(r->eflags, d.eflags);
}
GP4_IMPL(0x0053c2d1, FUN_0053c2d1, "regs(esi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+list_00549db9+pool_00549db9+textout_0054d211+script_all_00835846")
