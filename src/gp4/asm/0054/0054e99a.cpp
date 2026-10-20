#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x0054e95c with the menu action 0x0053d636: event edx = 1 clears bit 7 of the flag byte at
// esi + 0x1f and returns edx = 0 (flags of that `and`); event 5 runs 0x0053d636 with edx saved around it
// (push/pop), and leaving the registers and flags it produces; any other event only leaves the flags of
// `cmp edx, 5`.
// FUNCTION: GP4 0x0054e99a
void __cdecl FUN_0054e99a(gp4::Regs* r) {
    const uint32_t event = r->edx;
    if (event == 1u) {
        const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, r->esi, 0x1f) & 0x7fu);
        GP4_FIELD(uint8_t, r->esi, 0x1f) = cleared;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
        r->edx = 0u;
    } else if (event == 5u) {
        gp4::Regs c = *r;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 5u));
        gp4::call_regs(0x0053d636, c);
        c.edx = event;
        *r = c;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(event, 5u));
    }
}
GP4_IMPL(0x0054e99a, FUN_0054e99a, "regs(edx:u32{1,5,5,2,0,0xffffffff}, esi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0054eaf7+links_0054eaf7+tail_0054eb10+ctl_007ab548+msgbox_0053d636+script_0054d798+tid_0053d636")
