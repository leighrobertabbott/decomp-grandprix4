#include <gp4/gp4.h>
#include <gp4/flags.h>

// `call 0x0051b103 ; call 0x0051b2ea ; call 0x0051b094 ; ret`: stores the three words at esi as the
// second saved triple (0x00933a1a..), converts it (0x0051b2ea) into the 16-bit triple at
// 0x00933a14.. and copies that to edi. The callees keep every register; the flags are those
// 0x0051b2ea leaves (the copies touch none).
// FUNCTION: GP4 0x0051b190
void __cdecl FUN_0051b190(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0051b103, c);
    gp4::call_regs(0x0051b2ea, c);
    gp4::call_regs(0x0051b094, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051b190, FUN_0051b190, "regs(esi:ptr[0x10]:bytes, edi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
