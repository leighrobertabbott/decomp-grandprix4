#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-item text step (pushal wrapper), the sibling of 0x0053c135. From the item record esi it loads four
// zero-extended words into registers: ebx = word +4, eax = word +6, ebp = word +8, ecx = word +0xa. It
// inserts that entry into the sorted draw list (0x00549db9), latches the four values (0x0054cd42, which
// raises the flag byte 0x00833cf4), runs the empty routines 0x0054a1d7 and 0x0054d70a, stores ebx as the
// text cursor x (0x0054cbca), draws the string buffer 0x00823100 through the text output 0x0054d211 and
// clears the flag again (0x0054cd61). popal restores every register; the flags are those the last callee
// leaves.
// FUNCTION: GP4 0x0053c16a
void __cdecl FUN_0053c16a(gp4::Regs* r) {
    const uint32_t item = r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint16_t, item, 4);
    c.eax = GP4_FIELD(uint16_t, item, 6);
    c.ebp = GP4_FIELD(uint16_t, item, 8);
    c.ecx = GP4_FIELD(uint16_t, item, 0x0a);
    gp4::call_regs(0x00549db9, c);
    gp4::call_regs(0x0054cd42, c);
    gp4::call_regs(0x0054a1d7, c);
    gp4::call_regs(0x0054d70a, c);
    gp4::call_regs(0x0054cbca, c);
    c.esi = 0x00823100u;
    gp4::call_regs(0x0054d211, c);
    gp4::call_regs(0x0054cd61, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053c16a, FUN_0053c16a, "regs(esi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+textout_0054d211")
