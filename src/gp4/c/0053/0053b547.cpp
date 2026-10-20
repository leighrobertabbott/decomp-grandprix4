#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads eax = the word at 0x007c2e96 and draws the text for the item record in esi through
// 0x0053b570 (a pushal/popal routine). eax keeps the loaded word; the flags are those 0x0053b570
// leaves.
// FUNCTION: GP4 0x0053b547
void __cdecl FUN_0053b547(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint16_t, 0x007c2e96);
    gp4::call_regs(0x0053b570, c);
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053b547, FUN_0053b547, "regs(esi:ptr[0x10]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=textout_0054d211+trk_007ad894+cname_00838236+script_0053b570")
