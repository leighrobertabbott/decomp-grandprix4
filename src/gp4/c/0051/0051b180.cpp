#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts one 8-bit RGB triple at esi to hue/saturation/value and stores the 16-bit result at edi:
// 0x0051b0d4 scales the triple into the working words 0x00933a14.., 0x0051b1f9 turns them into
// hue/saturation/value (0x00933a1a..0x00933a1e) and 0x0051b0b4 copies those to edi. The registers are
// kept by every callee; the flags are those 0x0051b1f9 leaves (the copy touches none).
// FUNCTION: GP4 0x0051b180
void __cdecl FUN_0051b180(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0051b0d4, c);
    gp4::call_regs(0x0051b1f9, c);
    gp4::call_regs(0x0051b0b4, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051b180, FUN_0051b180, "regs(esi:ptr[0x10]:u8{0,0,1,2,0x40,0x80,0xff,3,7,0x20}, edi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
