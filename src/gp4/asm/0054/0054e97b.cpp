#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x0054e95c with the menu redraw 0x0053d6eb: event edx = 1 clears bit 7 of the flag byte at
// esi + 0x1f and returns edx = 0 (flags of that `and`); event 5 runs 0x0053d6eb with edx saved around it
// (push/pop), leaving the registers and flags it produces; any other event only leaves the flags of
// `cmp edx, 5`.
// FUNCTION: GP4 0x0054e97b
void __cdecl FUN_0054e97b(gp4::Regs* r) {
    const uint32_t event = r->edx;
    if (event == 1u) {
        const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->esi, 0x1f) & 0x7fu);
        GP4_FIELD(uint8_t, r->esi, 0x1f) = cleared;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
        r->edx = 0u;
    } else if (event == 5u) {
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 5u));
        gp4::call_regs(0x0053d6eb, c);
        c.edx = event;
        *r = c;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 5u));
    }
}
GP4_IMPL(0x0054e97b, FUN_0054e97b, "regs(edx:u32{1,5,5,2,0,0xffffffff}, esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0054eaf7+links_0054eaf7+tail_0054eb10+msgbox_0053d6eb+tid_0053d6eb")
